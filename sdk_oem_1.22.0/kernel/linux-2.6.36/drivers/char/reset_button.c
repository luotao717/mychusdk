/*
 * linux/arch/arm/mach-oxnas/reset_button.c
 *
 * Copyright (C) 2006 Oxford Semiconductor Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kobject.h>
#include <linux/workqueue.h>
#include <linux/vs_pio.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/vs_debug.h>
#include <asm/uaccess.h>
#include <asm/mach-ralink/surfboardint.h>

#include "ralink_gpio.h"

MODULE_LICENSE("GPL v2");

#define SWITCH_NUM          (VS_GPIO_RESET_KEY)

#define IRQ_NUM             SURFBOARDINT_GPIO

#define POWER_SWITCH_NUM    (VS_GPIO_POWER_INT_KEY)

//#define NET_SWITCH_NUM      VS_GPIO_NET_SWITCH

#define SWITCH_MASK         (1UL << (SWITCH_NUM))
#define VS_POWER_LED      (1UL << (VS_GPIO_SYS_LED - 24))
//#define NET_SWITCH_MASK     (1UL << (NET_SWITCH_NUM))

#define VS_RESET_GPIO_MODE  (1UL << 0)
#define VS_SPICS1_GPIO_MODE  (1UL << 12)
#define VS_WAN_MODE	(1UL << 13)
#define VS_LAN_MODE	(1UL << 15)

#define TIMER_INTERVAL_JIFFIES  ((HZ) >> 3) /* An eigth of a second */

extern spinlock_t vs_gpio_spinlock;

static struct timer_list timer;

static u32 vs_ralink_gpio_intp;
static u32 vs_ralink_gpio_edge;
static u32 vs_ralink_gpio3924_intp;
static u32 vs_ralink_gpio3924_edge;
static u32 vs_ralink_gpio7140_intp;
static u32 vs_ralink_gpio7140_edge;
static u32 vs_ralink_gpio72_intp;
static u32 vs_ralink_gpio72_edge;

typedef struct _vs_gpio_time_record_t {
	unsigned long falling;
	unsigned long rising;
} vs_gpio_time_record_t;

static vs_gpio_time_record_t vs_record;
static struct proc_dir_entry *printk_level_proc_file = NULL;


static char reset_debug =0 ;								//   1 : output debug  info.   0 : don't output debug info
#define dprintk(arg...) \
	do { if (reset_debug) printk(arg); } while (0)

/*start modifiedy by huangshiqiu on 2013-06-14*/
static vs_gpio_time_record_t vs_poweroff_record;
/*end modifiedy by huangshiqiu on 2013-06-14*/

/*start modifiedy by huhao on 2013-08-14*/
static u8  vs_power_state;
/*end modifiedy by huhao on 2013-08-14*/

/*start: modified by huang shiqiu on 2014-03-21*/
#if 0
/*start: modified by huang shiqiu on 2014-03-17*/
/*解决 : 复重按reset键后，系统会出现"Kernel panic - not syncing: Attempted to kill init!"问题*/
/*0表示用户按下reset按键后，会发信号量；1表示按下reset按键后，不通知上层应用程序*/
static int g_reset_key_flag = 0;
/*end: modified by huang shiqiu on 2014-03-17*/
#endif
/*解决 : 复重按reset键后，系统会出现"Kernel panic - not syncing: Attempted to kill init!"问题*/
/*0表示用户按下reset按键后，会发信号量；1表示按下reset按键后，不通知上层应用程序*/
/*默认值为1，表示按下reset按键后，不通知上层应用程序*/
static int g_reset_key_flag = 1;

/*启动一个25秒定时器，定时器到期后，配置为0，表示用户按下reset按键后，会发信号量。*/
struct timer_list g_reset_timer;
/*end: modified by huang shiqiu on 2014-03-21*/
int vs_printk_debug_level = VS_KERN_WARNING;

/* Proc file operations  */
static char ioreg_buf[256];
static struct proc_dir_entry *intreg_proc_file = NULL;

struct _vs_key_cdev {
    struct cdev *key_cdev;
    int key_event;                    //poll function flag : 1表示有中断事件
    int key_value;                    //get value : 0表示无效；1表示插入；2表示拔出
};

static dev_t vs_reset_tdev;
static int vs_reset_major  = 0;

#define VS_RESET_CDEV_NAME	     "vs_reset_key"
static struct _vs_key_cdev               *vs_reset_cdev;
static DECLARE_WAIT_QUEUE_HEAD(vs_reset_waitq);      //创建一个等待队列
static int button_on = 0;

void vs_btn_send_func_sig(int sig)
{
        struct task_struct *task_struct_p;

        local_irq_disable();
        for_each_process(task_struct_p) {
                if (task_struct_p->pid == 1) /* the init process */ {
                        force_sig(sig, task_struct_p);
                        break;
                }
        }
        local_irq_enable();
}
EXPORT_SYMBOL(vs_btn_send_func_sig);



/*
 * 1. save the PIOINT and PIOEDGE value
 * 2. clear PIOINT by writing 1
 * (called by interrupt handler)
 */
static void ralink_gpio_save_clear_intp(void)
{
        vs_ralink_gpio_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOINT));
        vs_ralink_gpio_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOEDGE));
        *(volatile u32 *)(RALINK_REG_PIOINT) = cpu_to_le32(0x00FFFFFF);
        *(volatile u32 *)(RALINK_REG_PIOEDGE) = cpu_to_le32(0x00FFFFFF);

	vs_ralink_gpio7140_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140INT));
	vs_ralink_gpio7140_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140EDGE));
        *(volatile u32 *)(RALINK_REG_PIO7140INT) = cpu_to_le32(0xFFFFFFFF);
        *(volatile u32 *)(RALINK_REG_PIO7140EDGE) = cpu_to_le32(0xFFFFFFFF);

	vs_ralink_gpio72_intp   = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72INT));
	vs_ralink_gpio72_edge   = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72EDGE));
        *(volatile u32 *)(RALINK_REG_PIO72INT) = cpu_to_le32(0x00FFFFFF);
        *(volatile u32 *)(RALINK_REG_PIO72EDGE) = cpu_to_le32(0x00FFFFFF);
}

/** Have to use active low level interupt generation, as otherwise might miss
 *  interrupts that arrive concurrently with a PCI interrupt, as PCI interrupts
 *  are generated via GPIO pins and std PCI drivers will not know that there
 *  may be other pending GPIO interrupt sources waiting to be serviced and will
 *  simply return IRQ_HANDLED if they see themselves as having generated the
 *  interrupt, thus preventing later chained handlers from being called
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
static void int_handler(unsigned int irq, struct irqaction *dev_id)
#else 
static irqreturn_t int_handler(int irq, void *dev_id)
#endif
{
    /*start modifiedy by huangshiqiu on 2013-06-14*/
    #if 0
	int status = IRQ_NONE;
    #endif
	int status = IRQ_HANDLED;
    /*end modifiedy by huangshiqiu on 2013-06-14*/
	unsigned long now;

	printk(KERN_INFO "enter gpio int_handler\n");

	ralink_gpio_save_clear_intp();

	now = jiffies;
#if 0
	printk(KERN_INFO "gpio_intp:0x%x, gpio_edge:0x%x, gpio7140_intp:0x%x, gpio7140_edge:0x%x, gpio72_intp:0x%x, gpio72_edge:0x%x\n",
              vs_ralink_gpio_intp, vs_ralink_gpio_edge, 
              vs_ralink_gpio7140_intp, vs_ralink_gpio7140_edge,
              vs_ralink_gpio72_intp, vs_ralink_gpio72_edge);
#endif

	/* Is the interrupt for user */
	/* modify by zhangguoquan on 2014-07-15*/

	if (vs_ralink_gpio_intp  & SWITCH_MASK) {
                 if (vs_ralink_gpio_edge & SWITCH_MASK)
		{ //rising edge
						printk(KERN_INFO "Get the reset rising interrupt\n");
		    	button_on = 0;
                        if ((vs_record.rising != 0) && time_before_eq(now, vs_record.rising + 40L)) {
                                /*
                                 * If the interrupt comes in a short period,
                                 * it might be floating. We ignore it.
                                 */
                        } else {
                                vs_record.rising = now;
                                if (time_before(now, vs_record.falling + 2*HZ)) {
					                /* Ignore one click */
                                } else {
                                    /*start: modified by huang shiqiu on 2014-03-17*/

                				//	    printk(KERN_ERR "Send the SIGUSR2\n");
                				//	    vs_btn_send_func_sig(SIGUSR2);
                				vs_reset_cdev->key_event = 1;
                				 wake_up_interruptible(&vs_reset_waitq);

                                        /*start: modified by huang shiqiu on 2014-03-21*/
                                        /*为防止SIG SIGUSR2信号量丢失，20秒后，再次使用发送SIGUSR2功能*/
                                        //mod_timer(&g_reset_timer, jiffies+20*HZ);
                                        /*end: modified by huang shiqiu on 2014-03-21*/

                                    /*end: modified by huang shiqiu on 2014-03-17*/
                                }
                        }
                } else { //falling edge
                    printk(KERN_INFO "Get the reset falling interrupt\n");
                    vs_record.falling = now;
		    button_on = 1;

		        }
	}

	return status;
}


static int intreg_read_proc(char *page, char **start, off_t off,
                            int count, int *eof, void *data)
{
        int len = 0;
        off_t begin = 0;

	memset(ioreg_buf, 0, sizeof(ioreg_buf));
	len += sprintf(ioreg_buf, "press:%d\n", button_on);
#if 0
	len += sprintf(ioreg_buf, "Interrupt register:\n");
	len += sprintf(ioreg_buf+len, "RALINK_REG_IRQ0STAT=0x%x\n", 
	               *(volatile unsigned int *)RALINK_REG_IRQ0STAT);	
	len += sprintf(ioreg_buf+len, "RALINK_REG_IRQ1STAT=0x%x\n",
		       *(volatile unsigned int *)RALINK_REG_IRQ1STAT);
	len += sprintf(ioreg_buf+len, "RALINK_REG_INTTYPE=0x%x\n",
		       *(volatile unsigned int *)RALINK_REG_INTTYPE); 
	len += sprintf(ioreg_buf+len, "RALINK_REG_RAWINTSTAT=0x%x\n",
		       *(volatile unsigned int *)RALINK_REG_RAWINTSTAT);
	len += sprintf(ioreg_buf+len, "RALINK_REG_INTENA=0x%x\n",
		       *(volatile unsigned int *)RALINK_REG_INTENA);
        len += sprintf(ioreg_buf+len, "RALINK_REG_INTDIS=0x%x\n",
                       *(volatile unsigned int *)RALINK_REG_INTDIS);

	len += sprintf(ioreg_buf+len, "GPIO register:\n");
        len += sprintf(ioreg_buf+len, "RALINK_REG_PIOINT=0x%x\n",
                       *(volatile unsigned int *)RALINK_REG_PIOINT);
        len += sprintf(ioreg_buf+len, "RALINK_REG_PIOEDGE=0x%x\n",
                       *(volatile unsigned int *)RALINK_REG_PIOEDGE);
	len += sprintf(ioreg_buf+len, "RALINK_REG_PIORENA=0x%x\n",
		       *(volatile unsigned int *)RALINK_REG_PIORENA);
	len += sprintf(ioreg_buf+len, "RALINK_REG_PIOFENA=0x%x\n",
		       *(volatile unsigned int *)RALINK_REG_PIOFENA);
	len += sprintf(ioreg_buf+len, "RALINK_REG_PIODATA=0x%x\n",
		       *(volatile unsigned int *)RALINK_REG_PIODATA);
#endif
	len = 0;
        len += sprintf(page, "%s", ioreg_buf);
        *eof = 1;
        
        if (off >= len + begin)
                return 0;
        
        *start = page + (off - begin); 
        return ((count < begin + len - off) ? count : begin + len - off);
}

struct irqaction vs_reset_irqaction = {
        .handler = int_handler,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
        .flags = IRQF_DISABLED,
#else                   
        .flags = SA_INTERRUPT,                  
#endif                  
        .name = "Reset Button",
}; 

/*start: modifiedy by huangshiqiu on 2014-03-21*/
static void vs_reset_key_timer_handler(unsigned long arg)
{
	printk(KERN_INFO "enable send the SIGUSR2\n");

    /*0表示用户按下reset按键后，会发信号量*/
    g_reset_key_flag = 0;
    
    return;
}
/*end: modifiedy by huangshiqiu on 2014-03-21*/

static unsigned int vs_reset_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0; 					//用来记录发生的事件，以unsigned int类型返回


    poll_wait(filp, &vs_reset_waitq, wait); 	

    if (vs_reset_cdev->key_event == 1)
    {   
        printk(KERN_INFO "[func:%s, line:%d] reset poll wake up\n", __FUNCTION__, __LINE__);
    	mask |= POLLIN|POLLRDNORM; 			//中断事件发生，这时有数据可读，在mask中标记是可读事件发生
    }
	
	vs_reset_cdev->key_event = 0;
    return mask; 							//返回事件记录，返回0则表示等待事件超时
}

static int vs_reset_open(struct inode *inode, struct file *filp)
{	
	return 0;

}

static int vs_reset_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations vs_reset_fops = 
{
	.owner = THIS_MODULE,
	.open = vs_reset_open,
	.release = vs_reset_release,
	.poll = vs_reset_poll,
};

static int vs_atoi(const char *s)
{
	int val = 0;

	for (;; s++) {
		switch (*s) {
			case '0'...'9':
			val = 10*val+(*s-'0');
			break;
		default:
			return val;
		}
	}
}

static int printk_debug_level_read_proc(char *page, char **start, off_t off,
                         int count, int *eof, void *data)
{
        int len;
        if (off > 0) {
                    *eof = 1;
                    return 0;
        }
        len = sprintf(page, "%d\n", vs_printk_debug_level);

        return len;
}

static ssize_t printk_debug_level_write_proc(struct file* file, const char* buffer,
                             unsigned long count, void *data)
{
	char * foo = kmalloc(count, GFP_ATOMIC);

	if(!foo){
		printk(KERN_ERR "[file:%s line:%d] vs_printk_debug_level: kmalloc is failed, out of memory.\n", __FILE__, __LINE__);
		return count;
	}

	if(copy_from_user(foo, buffer, count)) {
		kfree (foo);		
		printk(KERN_ERR "[file:%s line:%d] vs_printk_debug_level: copy_from_user is failed.\n", __FILE__, __LINE__);
		return -EFAULT;
	}

	vs_printk_debug_level = vs_atoi(foo);
	kfree (foo);

	printk(KERN_INFO "[file:%s line:%d] vs_printk_debug_level = %d\n", 
		__FILE__, __LINE__, vs_printk_debug_level);

	return count;
}


static int __init reset_button_init(void)
{
	u32 gpiomode,tem;
	int result = 0;
	static struct class *reset_button_class;

	reset_button_class = class_create(THIS_MODULE, "reset_class");
	if (IS_ERR(reset_button_class))
    	{
        	printk("Err:failed in creating class.\n");
        	return -1;
    	}

	
	/*创建复位字符设备*/
	vs_reset_cdev = (struct _vs_key_cdev*)kmalloc(sizeof(struct _vs_key_cdev), GFP_KERNEL);
    if (vs_reset_cdev == NULL) 
    {
        printk(KERN_ERR "[func:%s, line:%d] kmalloc failed!\n", __FUNCTION__, __LINE__);
        goto error_01;
    }
	memset((void *)vs_reset_cdev, 0, sizeof(struct _vs_key_cdev));
	
	vs_reset_tdev= MKDEV(vs_reset_major, 0);
	if(vs_reset_major)
    		result = register_chrdev_region(vs_reset_tdev, 1, VS_RESET_CDEV_NAME);
	else
		result = alloc_chrdev_region(&vs_reset_tdev, 0, 1, VS_RESET_CDEV_NAME);
    	if (result < 0) {
        	printk(KERN_WARNING "fail allocate chrdev\n");
        	goto error_02;
    	}else{
        	vs_reset_major = MAJOR(vs_reset_tdev);
    }

	vs_reset_cdev->key_cdev = cdev_alloc();
	if (vs_reset_cdev->key_cdev == NULL)
    	{
       	 	printk(KERN_ERR "[func:%s, line:%d] cdev_alloc failed!\n", __FUNCTION__, __LINE__);
        	goto error_03;
    	}
	cdev_init(vs_reset_cdev->key_cdev, &vs_reset_fops);

	result = cdev_add(vs_reset_cdev->key_cdev, vs_reset_tdev, 1);
	 if (result != 0) 
    	{
        	printk(KERN_ERR "Error adding character device\n");
        	goto error_03;
    	}

	device_create(reset_button_class, NULL, vs_reset_tdev, NULL, VS_RESET_CDEV_NAME);
	
	/***********creat get mac dev*****************/
	/*********************************************/

	
	/* Set the proc for interrupt registers */
	    intreg_proc_file  = create_proc_entry("intreg", 0, NULL);
	    if (intreg_proc_file == NULL) {
	            printk(KERN_INFO "intreg_proc_file: Couldn't create interrupt register proc entry\n");
	            return -ENOMEM;
	    }
	    intreg_proc_file->read_proc = intreg_read_proc; /* reader function */

	printk_level_proc_file = create_proc_entry("vs_printk_debug_level", 0, NULL);
	if (printk_level_proc_file == NULL) {
		printk(KERN_INFO "vs_printk_level: Couldn't create printk debug level proc entry\n");
		return -ENOMEM;
	}
	printk_level_proc_file->read_proc  = printk_debug_level_read_proc; /* reader function */
	printk_level_proc_file->write_proc = printk_debug_level_write_proc; /* writer function */
	/* Set the interrupt registers   */

	gpiomode = le32_to_cpu(*(volatile u32*)(RALINK_REG_GPIOMODE));
	gpiomode |= VS_RESET_GPIO_MODE | RALINK_GPIOMODE_DFT ;
//	gpiomode &= ~( VS_WAN_MODE | VS_LAN_MODE );
	*(volatile u32*)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);

	tem = le32_to_cpu(*(volatile u32*)(RALINK_REG_PIODIR));
	tem &= ~( SWITCH_MASK);
	*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tem);
	
	*(volatile unsigned int *)RALINK_REG_PIOEDGE = (SWITCH_MASK);
	*(volatile unsigned int *)RALINK_REG_PIORENA = (SWITCH_MASK);
        *(volatile unsigned int *)RALINK_REG_PIOFENA = (SWITCH_MASK);
 #if 0   
	tem = le32_to_cpu(*(volatile u32*)(RALINK_REG_PIO3924DIR));
	tem |= VS_POWER_LED ;
	*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tem);

	*(volatile u32 *)(RALINK_REG_PIO3924RESET) = 	VS_POWER_LED;
#endif
	/* Install a shared interrupt handler on the appropriate GPIO bank's
	   interrupt line */
	setup_irq(SURFBOARDINT_GPIO, &vs_reset_irqaction);

	vs_ralink_gpio_intp = 0;
	vs_ralink_gpio_edge = 0;
	vs_ralink_gpio7140_intp = 0;
	vs_ralink_gpio7140_edge = 0;
	vs_ralink_gpio72_intp = 0;
	vs_ralink_gpio72_edge = 0;

	memset(&vs_record, 0, sizeof(vs_record));

    /*start modifiedy by huangshiqiu on 2013-06-14*/
	memset(&vs_poweroff_record, 0, sizeof(vs_poweroff_record));
    /*end modifiedy by huangshiqiu on 2013-06-14*/

    /*start: modifiedy by huangshiqiu on 2014-03-21*/
	printk("start reset key timer.\n");
    setup_timer(&g_reset_timer, vs_reset_key_timer_handler, 0);
    g_reset_timer.expires = jiffies+25*HZ;
    add_timer(&g_reset_timer);
    /*end: modifiedy by huangshiqiu on 2014-03-21*/

	printk(KERN_INFO "Reset button driver registered\n");
	return 0;

error_04:
    cdev_del(vs_reset_cdev->key_cdev);
error_03:
    unregister_chrdev_region(MKDEV(vs_reset_major, 0), 1);
error_02:
    kfree(vs_reset_cdev);
error_01:
    remove_proc_entry("intreg", NULL);
    return -ENOMEM;  

}

static void __exit reset_button_exit(void)
{
	remove_proc_entry("intreg", NULL);
        unregister_chrdev_region( vs_reset_tdev, 1);
	
    /*start: modified by huang shiqiu on 2014-04-19*/
	/* Remove the handler for the shared interrupt line */
	free_irq(SURFBOARDINT_GPIO, &vs_reset_irqaction);
    /*end: modified by huang shiqiu on 2014-04-19*/
    
	/* Deactive the timer */
	del_timer_sync(&timer);

    /*start: modifiedy by huangshiqiu on 2014-03-21*/
    if (timer_pending(&g_reset_timer))
    {
        //stop g_reset_timer
        del_timer(&g_reset_timer);
    }
    /*start: modifiedy by huangshiqiu on 2014-03-21*/

}

/** 
 * macros to register intiialisation and exit functions with kernal
 */
module_init(reset_button_init);
module_exit(reset_button_exit);



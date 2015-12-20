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

/*用于保护g_ap_get_phone_mac_addr缓冲区，由于此缓冲区会有定时器中更新，所以，只能采用这种方式。
 *1表示上层正在获取mac地址列表，这时，WIFI驱动不能更新mac地址列表；
 *0表示上层应用没有获取mac地址，WIFI驱动可能继发续更新mac地址列表。
 */
volatile int g_get_mac_emaphore = 0;

/*WIFI驱动保存mac地址列表的缓冲区*/
hi_ap_get_phone_mac_addr	g_ap_get_phone_mac_addr; 
int VS_ABLE_PHMAC_FLAG = 0;
/*start modifiedy by zhangguoquan on 2014-08-04*/
static DECLARE_WAIT_QUEUE_HEAD(vs_phmac_waitq);      //创建一个等待队列
static int vs_phmac_event =0;
static dev_t vs_phmac_tdev;
static int vs_phmac_major = 0;
static struct cdev *vs_phmac_cdev;
#define  PHONE_MAC_NAME   "vs_app_get_phone_mac"
static struct timer_list  vs_phmac_timer;


/*WIFI驱动直接调用此函数，将获取的mac地址直接保存到g_ap_get_phone_mac_addr缓冲区*/
void vs_save_phone_mac_mac(unsigned char *mac_addr, char ssi)
{
	if(VS_ABLE_PHMAC_FLAG == 1){
			static int first_time = 1;
			int n=0, flag=0;
		    /*只有g_get_mac_flag为零后，才能进行下面的操作；避免访问mac列表缓冲区发生冲突*/
		    while (1 == g_get_mac_emaphore){
				n++;
				udelay(10);
				if(n >100){
					PRINTK_FOR_DEBUG(VS_KERN_ERR,"[vs_save_phone_mac_mac]some procedure are using phmac\n");
					return;
				}
			}
																		    
		    
		    /*保护mac列表缓冲区*/
		    g_get_mac_emaphore = 1;
		    
		    if (g_ap_get_phone_mac_addr.mac_num < (HI_MAC_BUF_SIZE / HI_MAC_LEN) )
		    {
		//    		printk("mac start jiffes=%ld",jiffies);
		/*		for(n=0; n<g_ap_get_phone_mac_addr.mac_num; n++)
				{
					if(strncmp(mac_addr,(unsigned char *)&(g_ap_get_phone_mac_addr.mac_addr[n* HI_MAC_LEN]) , HI_MAC_LEN) == 0)
					{
						flag = 1;
						break;
					}
				}*/
		//		printk("mac zhong jiffes=%ld",jiffies);
		//		if(flag == 0){
					 memcpy((unsigned char *)&(g_ap_get_phone_mac_addr.mac_addr[g_ap_get_phone_mac_addr.mac_num * HI_MAC_LEN]), \
					 				mac_addr, HI_MAC_LEN-1);

		      			  g_ap_get_phone_mac_addr.mac_num++;
					g_ap_get_phone_mac_addr.mac_addr[g_ap_get_phone_mac_addr.mac_num * HI_MAC_LEN-1] = (unsigned char)ssi;
								 
					 PRINTK_FOR_DEBUG(VS_KERN_DEBUG, "MAC = %02x:%02x:%02x:%02x:%02x:%02x, ssi=%d\n",mac_addr[0],mac_addr[1],
		 			                                       mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],ssi);
		//		}
		//		printk("mac end jiffes=%ld",jiffies);
		    }
		    /*取消mac列表缓冲区*/
		    g_get_mac_emaphore = 0;
			if(first_time == 1){
				add_timer(&vs_phmac_timer);
				first_time = 0;
			}
			else{
				if(timer_pending(&vs_phmac_timer) != 1)
				mod_timer(&vs_phmac_timer,  jiffies+2*HZ);
			}
	}
    return;    
}
EXPORT_SYMBOL(vs_save_phone_mac_mac);

void vs_get_phone_mac_list(hi_ap_get_phone_mac_addr *app_buf)
{
    /*只有g_get_mac_flag为零后，才能进行下面的操作；避免访问mac列表缓冲区发生冲突*/
    while (1 == g_get_mac_emaphore);
    
    /*保护mac列表缓冲区*/
    g_get_mac_emaphore = 1;
    
    /*直接将mac列表缓冲区全部拷贝给上层应用程序*/
    memcpy((unsigned char *)app_buf, (unsigned char *)&g_ap_get_phone_mac_addr, sizeof(g_ap_get_phone_mac_addr));
    
    /*上层应用程序获取当前的mac列表缓冲区后，将此计数器清零；后续WIFI驱动将重新从零开始计数*/
    g_ap_get_phone_mac_addr.mac_num = 0;
    
    /*取消mac列表缓冲区*/
    g_get_mac_emaphore = 0;
    
    return;
}


static unsigned int vs_phmac_poll (struct file *filp, struct poll_table_struct *wait)
{
	int mack = 0;
	poll_wait(filp, &vs_phmac_waitq, wait);
	if(vs_phmac_event == 1)
	{
		printk(KERN_INFO "[func:%s, line:%d] phone MAC wake up\n", __FUNCTION__, __LINE__);
		mack |= POLLIN |POLLRDNORM;
	}
	vs_phmac_event = 0;
	return mack;
}

static ssize_t vs_phmac_read (struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	int err,num = 0;
	unsigned int copy_size;
	copy_size = count >(g_ap_get_phone_mac_addr.mac_num*HI_MAC_LEN)? g_ap_get_phone_mac_addr.mac_num*HI_MAC_LEN: count;
	while (1 == g_get_mac_emaphore){
			num++;
		udelay(10);
		if(num>100){
			PRINTK_FOR_DEBUG(VS_KERN_ERR,"[vs_phmac_read]some procedure are using phmac\n");
			g_ap_get_phone_mac_addr.mac_num = 0;
			return 0;
		}
	}

	if(g_ap_get_phone_mac_addr.mac_num == 0)
		return 0;
	g_get_mac_emaphore = 1;

	err = copy_to_user(buff, (char *)g_ap_get_phone_mac_addr.mac_addr, copy_size);
	if(err)
	{
		 printk(KERN_ERR "[func:%s, line:%d] copy_to_user is failed\n", __FUNCTION__, __LINE__);
		 g_get_mac_emaphore = 0;
		return -EFAULT;
	}
	for(err=0;err<copy_size/HI_MAC_LEN; err++)
	{
		 PRINTK_FOR_DEBUG(VS_KERN_DEBUG, "READ_MAC = %02x:%02x:%02x:%02x:%02x:%02x, ssi= %d\n",(unsigned char )buff[err*HI_MAC_LEN+0], 
		 					(unsigned char )buff[err*HI_MAC_LEN+1], (unsigned char )buff[err*HI_MAC_LEN+2], \
		 					(unsigned char )buff[err*HI_MAC_LEN+3], (unsigned char )buff[err*HI_MAC_LEN+4], \
		 					(unsigned char )buff[err*HI_MAC_LEN+5], buff[err*HI_MAC_LEN+6]);
	}
	 g_ap_get_phone_mac_addr.mac_num = 0;
	 g_get_mac_emaphore = 0;
//	 printk("read end jiffes=%ld",jiffies);
	return copy_size/HI_MAC_LEN;

/*	while (1 == g_get_mac_emaphore);
	g_get_mac_emaphore = 1;
	memcpy((unsigned char *)buff, (unsigned char *)&g_ap_get_phone_mac_addr.mac_addr, strlen(g_ap_get_phone_mac_addr.mac_addr));
	 g_ap_get_phone_mac_addr.mac_num = 0;
	 g_get_mac_emaphore = 0;
	 return strlen(g_ap_get_phone_mac_addr.mac_addr);
*/

}

static int vs_phmac_open (struct inode *inode, struct file *filp)
{
	VS_ABLE_PHMAC_FLAG = 1;
	g_ap_get_phone_mac_addr.mac_num = 0;
	return 0;
}
static int vs_phmac_close (struct inode *inode, struct file *filp)
{
	VS_ABLE_PHMAC_FLAG = 0;
	del_timer_sync(&vs_phmac_timer);
	return 0;
}

static struct file_operations phone_mac = {
	.open	 = vs_phmac_open,
	.release = vs_phmac_close,
	.read	 = vs_phmac_read,
	.poll	 = vs_phmac_poll,
};

 void vs_phmac_tfunc(unsigned long data)
 {
	   /*唤醒设备阻塞队列*/
  	vs_phmac_event = 1;
 	 wake_up_interruptible(&vs_phmac_waitq);
 }



static int __init  vs_get_mac_init(void)
{
	u32 gpiomode,tem;
	int result = 0;
	static struct class *get_mac_class;

	get_mac_class = class_create(THIS_MODULE, "get_mac_class");
	if (IS_ERR(get_mac_class))
    	{
        	printk("Err:failed in creating class.\n");
        	return -1;
    	}
	
	vs_phmac_tdev = MKDEV(vs_phmac_major, 0);
	
	result= alloc_chrdev_region(&vs_phmac_tdev,0,  1, PHONE_MAC_NAME);
	if(result != 0)
	{
		printk(KERN_WARNING "FIALE allocate phone mac chrdev\n" );
		return -ENOMEM; 
	}
	vs_phmac_cdev = cdev_alloc();
	if(vs_phmac_cdev == NULL)
	{
		printk(KERN_ERR "[func:%s,line:%d cdev_alloc failed\n]",__FUNCTION__,__LINE__);
		unregister_chrdev_region(vs_phmac_tdev , 1);	
		return -ENOMEM; 
	}
	cdev_init(vs_phmac_cdev, &phone_mac);
	result = cdev_add(vs_phmac_cdev, vs_phmac_tdev, 1);
	if(result != 0)
	{
		printk(KERN_ERR "Error adding character device\n");
		unregister_chrdev_region(vs_phmac_tdev , 1);	
		return -ENOMEM; 
	}
	init_timer(&vs_phmac_timer);
	vs_phmac_timer.expires = jiffies+2*HZ;
	setup_timer(&vs_phmac_timer,vs_phmac_tfunc,0);	

	device_create(get_mac_class, NULL, vs_phmac_tdev , NULL, PHONE_MAC_NAME);


	
 }

static void __exit vs_get_mac_exit(void)
{
	remove_proc_entry("intreg", NULL);
	unregister_chrdev_region( vs_phmac_tdev, 1);


}

/** 
 * macros to register intiialisation and exit functions with kernal
 */
module_init( vs_get_mac_init);
module_exit( vs_get_mac_exit);





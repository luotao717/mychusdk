/*
 * Copyright (C) 2009 by IOVST
 * 
 * File: vs_pio.h
 * 
 * Date: 2009-01-15
 *
 * Author: liuyong<liuyong@iovst.com>
 *
 * Version: 0.1
 *
 * Descriptor:
 *   vs_pio.h header file for pio driver
 *
 * Modified:
 *   2009-01-07 added the led flicker function
 */
#ifndef _VS_PIO_H
#define _VS_PIO_H

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <linux/delay.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/signal.h>
#include <linux/apm_bios.h>
#include <linux/vs_debug.h>
/**********************************************************
 * These GPIO define
 **********************************************************/

#define VS_GPIO_POWER_INT_KEY   (44)		//modify to 44 Hardware has changed
#define VS_GPIO_WIFI_LED        (72)    /*WIFI¿¿µÆ¼¿ÐT7620N·½°¸µÄè*/
#define VS_GPIO_INET_BLUE_LED   (40)

#define VS_GPIO_WPS_KEY         (12)    /*WPS KEY£¬Input£¬low active*/
#define VS_GPIO_RESET_KEY       (1)    /*RESET KEY,¼¿ÐT7620N·½°¸µÄè£¬Input£¬low active, ³¤°´5S¼ìµ½µ¿ç£¬¿¿»¿´¿È¿¿£¬¿¿¿¿µÆÁ¸£¬WEB¿ÃÌ¿»¿´µ¹¼Æ±*/
#define VS_GPIO_NET_SWITCH   (14)    /*SD CARD backup key, Input, low active, ½«SD CARDÄµÄ¿þ±¸·¿½ÄÖHDD*/
#define VS_GPIO_SYS_LED        (38)


//#define VS_RESET_DEV_NUM	110

/**********************************************************
 * These micros for the ioctl's cmd code
 **********************************************************/
#define STATUS_LED_CTL         (0x01)
#define FAN_CTL                (0x02)
#define HDD_LED_CTL            (0x03)
#define BEEP_CTL               (0x04)
#define POWER_CTL              (0x05)
#define WIFI_LED_CTL           (0x06)
#define INTERNET_LED_CTL       (0x07)

/**********************************************************
 * These micros for the control of the board's LEDs.
 * Value to control LEDs as ioctl's arg param.
 **********************************************************/
#define VS_LED_ON              0
#define VS_LED_OFF             1
#define VS_FLICKER             2
#define VS_NOFLICKER           3

/* wifi                 */
#define VS_WIFI_ON             0
#define VS_WIFI_OFF            1
#define VS_WIFI_FLICKER        2
#define VS_WIFI_NOFLICKER      3

/* internet             */
#define VS_INTERNET_BLUE_ON    0
#define VS_INTERNET_BLUE_OFF   1
#define VS_INTERNET_FLICKER    2
#define VS_INTERNET_NOFLICKER  3
#define VS_INTERNET_RED_ON     4
#define VS_INTERNET_RED_OFF    5

/* define LED           */
#define VS_STATUS_LED          1
#define VS_WIFI_LED            2
#define VS_INTERNET_LED        3

/***********************************************************
 ** These micros control this driver's operation.
 ************************************************************/
#define VS_LEN_NUM             2
#define VS_JMP_NUM             2
#define RANDOM_DEV_NUM         0
#define VS_PIO_DEV_NUM         100
#define VS_PIO_DBG             0
#define VS_PIOB_NUM            3

/***********************************************************
 * This structure sign whether the flicker is on or off
 * @flk_num:
 *          1 flicker
 *          0 no flicker
 ***********************************************************/
typedef struct vs_led_flicker_struct {
	spinlock_t flk_lock;
	int flk_num;
} vs_flk_t;

#define INIT_FLK(flk) \
	do {	      \
		flk.flk_lock = SPIN_LOCK_UNLOCKED;	\
		flk.flk_num = 0;		\
	} while (0)

/*start modifiedy by huangshiqiu on 2013-01-30*/
#define VS_DISK_SPIN_DOWN_TIMER			180  /*3¡¤??¨®*/

typedef struct vs_disk_spindown_struct {
	spinlock_t vs_disk_spin_lock;

	struct timer_list vs_disk_spindown_timer;	/*¨®2?¨¬DY???¡§???¡Â¡ê??¨¹?¨²?a1???¡ê*/
	unsigned int vs_timer_cnt;					/*?¡§¨º¡À?¡Â¦Ì???¨ºy?¡Â*/
	unsigned int vs_disk_spindown_flag;			/*¨®2?¨¬¦Ì?DY??¡À¨º¨º?¡ê?1¡À¨ª¨º?DY??*/

	unsigned int vs_disk_rw_cnt;				/*¨®?¨®¨²1?¡À¨¹¡¤¡é?¨ª¨®2?¨¬DY???¨¹¨¢?¨º¡À¡ê??¨¹?¨¢??D?¨®2?¨¬¦Ì??¨º¨¬a?¡ê2013-02-19*/
} vs_disk_spindown_t;

#define INIT_DISK_SPINDOWN(arg) \
	do {	      \
		arg.vs_disk_spin_lock = SPIN_LOCK_UNLOCKED;\
        arg.vs_timer_cnt = 0;\
        arg.vs_disk_spindown_flag = 0;		\
        arg.vs_disk_rw_cnt = 0;		\
	} while (0)
/*end modifiedy by huangshiqiu on 2013-01-30*/

/**********************GET MAC****************************/
#if defined(CONFIG_GET_MAC)

#define HI_MAC_LEN                  7
#define HI_MAC_BUF_SIZE             11900

/*´Ë½á¹¹Ìå£¬ÐèÒª¶ÔÍâ¿ª·Å : WIFIÇý¶¯±£´æmacµØÖ·ÁÐ±íµÄ»º³åÇø*/
typedef struct __hi_ap_get_phone_mac_addr {
    volatile int    mac_num;                       /*±íÊ¾µ±Ç°»ñÈ¡ÁË¶àÉÙ¸öÓÐÓÃµÄmacµØÖ·*/
    unsigned char   mac_addr[HI_MAC_BUF_SIZE];     /*macµØÖ·ÁÐ±í»º³åÇø£»Ò»´Î×î¶à¿ÉÒÔ»ñÈ¡170¸ömacµØÖ·*/
}hi_ap_get_phone_mac_addr;

void vs_save_phone_mac_mac(unsigned char *mac_addr, char ssi);
void vs_get_phone_mac_list(hi_ap_get_phone_mac_addr *app_buf);

#endif
/***********************************************************/

extern int vs_printk_debug_level;


/***********************************************************
 * Functions
 ***********************************************************/
extern void vs_gpio_a_set_val(unsigned char pin, unsigned char high);
extern void vs_led_operation(unsigned int led, unsigned int ops);

extern void vs_btn_send_terminal_sig(void);
extern void vs_btn_send_func_sig(int sig);

#endif

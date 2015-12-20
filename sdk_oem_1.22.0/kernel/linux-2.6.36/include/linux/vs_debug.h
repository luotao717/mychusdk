/*
 * Copyright (C) 2013 by i4season
 * 
 * File: vs_debug.h
 * 
 * Date: 2013-01-31
 *
 * Author: huangshiqiu<huangshiqiu@i4season.com>
 *
 * Version: 0.1
 *
 * Descriptor:
 *   vs_debug.h header file for debug kernel driver
 *
 * Modified:
 *   2013-01-31 create this file
 */
 
#ifndef _VS_DEBUG_H
#define _VS_DEBUG_H
extern int vs_printk_debug_level;
/***********************************************************
 ** These is debug print micros.
 ************************************************************/
/*start modifiedy by huangshiqiu on 2013-01-30*/
#define	VS_KERN_EMERG	0	/* system is unusable			*/
#define	VS_KERN_ALERT	1	/* action must be taken immediately	*/
#define	VS_KERN_CRIT	2	/* critical conditions			*/
#define	VS_KERN_ERR		3	/* error conditions			*/
#define	VS_KERN_WARNING	4	/* warning conditions			*/
#define	VS_KERN_NOTICE	5	/* normal but significant condition	*/
#define	VS_KERN_INFO	6	/* informational			*/
#define	VS_KERN_DEBUG	7	/* debug-level messages			*/

#define VS_DEBUG_PRINTK_RAW(debug_level, fmt)    \
	do{ 								  \
		if (debug_level <= vs_printk_debug_level)		\
		{								\
			printk fmt; 			  \
		}								\
	}while(0)

#define VS_DEBUG_PRINTK(debug_level, fmt)   VS_DEBUG_PRINTK_RAW(debug_level, fmt)

/*start modifiedy by zhangguoquan on 2014-08-05*/
#define PRINTK_FOR_DEBUG(debug_level, arg...)  \
          do{		\
		  	if(debug_level <= vs_printk_debug_level) 	\
		  	{				\
				printk(arg);		\
		  	}				\
		}while(0)

/*end modifiedy by zhangguoquan on 2014-08-05*/
/*end modifiedy by huangshiqiu on 2013-01-30*/

#endif  /*_VS_DEBUG_H*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/proc_fs.h>
#include <net/netlink.h>
#include "sys_event_netlink.h"


extern struct net init_net;
extern struct sock *sys_event_nl_sk;

int mtk_nl_send_msg(void *msg, int msg_size)
{
	 int ret = 0;
	 struct sk_buff *skb = NULL;
	 struct nlmsghdr *nlh = NULL;
	 char *pmsg = (char *)msg;

	 if(msg == NULL || 0 == msg_size)
	 {
		printk("msg is null !\n");
		return ret;
	 }

	 if (NULL == sys_event_nl_sk)
	 {
	 	 printk("nl socket is null !\n");
	 	 return ret;
	 }

	 skb = nlmsg_new(msg_size, GFP_ATOMIC);
	 if (!skb)
	 {
		printk("%s:alloc skb failed\n", __func__);
		return ret;
	 }

	 nlh = nlmsg_put(skb, 0, 0, 0, msg_size, 0);

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
	 NETLINK_CB(skb).creds.pid = 0;
#else
	 NETLINK_CB(skb).pid = 0;
#endif
	 NETLINK_CB(skb).dst_group = 1;

	 memcpy(NLMSG_DATA(nlh),pmsg, msg_size);
	 ret = netlink_broadcast(sys_event_nl_sk, skb, 0, 1, GFP_ATOMIC);
	 
	 /*printk("%d bytes has been sent successfully !\n", ret);*/

	 return ret;
}

EXPORT_SYMBOL(mtk_nl_send_msg);

 /**************************************************************
 * Function    : str2Mac()
 * Abstract : Translate MAC ("00:01:02:03:04:05") -> MAC (mac[6])
 ***************************************************************/
int str2Mac (unsigned char *str, unsigned char *mac)
{
	 int i, c;

	 if (NULL == str || NULL == mac)
	 {
	 	 return FALSE;
	 }
    
	 for (i = 0; i < 6; i++)
	 {    
	 	 if (*str == '-' || *str == ':')
	 	 {
	 	 	 str++;
	 	 }

	 	 if (*str >= '0' && *str <= '9')
 	 	 {
	 	 	 c  = (unsigned char) (*str++ - '0'); 
	 	 }
	 	 else if (*str >= 'a' && *str <= 'f')
	 	 {
	 	 	 c  = (unsigned char) (*str++ - 'a') + 10;
	 	 }
	 	 else if (*str >= 'A' && *str <= 'F')
	 	 {
	 	 	 c  = (unsigned char) (*str++ - 'A') + 10; 
	 	 }
	 	 else
	 	 {
	 	 	 return FALSE;
	 	 }

	 	 c <<= 4;

	 	 if (*str >= '0' && *str <= '9')
	 	 {
	 	 	 c |= (unsigned char) (*str++ - '0');
	 	 }
	 	 else if (*str >= 'a' && *str <= 'f')
	 	 {
	 	 	 c |= (unsigned char) (*str++ - 'a') + 10;
	 	 }
	 	 else
	 	 {
	 	 	 c |= (unsigned char) (*str++ - 'A') + 10;
	 	 }

	 	 mac[i] = (unsigned char) c;

	 }

	 return TRUE;
}

 int mac2Str (unsigned char *mac, unsigned char *str, int bColon)
 {
	 if (NULL == mac || NULL == str)
	 {
	 	 return FALSE;
	 }
	 if (bColon)
	 {
	 	 sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", 
	 	 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	 }
	 else
	 {
	 	 sprintf(str, "%02X-%02X-%02X-%02X-%02X-%02X",
	 	 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	 }
 	 return TRUE;
 }

EXPORT_SYMBOL(str2Mac);
EXPORT_SYMBOL(mac2Str);

struct sock * init_sys_nl(void)
{
/*SOCK 由ali asec模块创建，因此这里需要屏蔽掉*/
#if 0
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
	struct netlink_kernel_cfg cfg = {
		.input	= NULL,
		.groups	= 0,
	};
	return netlink_kernel_create(&init_net, NETLINK_SYS_MSG, &cfg);
#else
	return netlink_kernel_create(&init_net, NETLINK_SYS_MSG, 0, NULL, NULL, THIS_MODULE);
#endif
#else
	return NULL;
#endif
}

struct sock *iot_rtk_eventd_netlink_sock_set(struct sock *nl_sk) 
{
    sys_event_nl_sk = nl_sk;
    return sys_event_nl_sk;
}
EXPORT_SYMBOL(iot_rtk_eventd_netlink_sock_set);



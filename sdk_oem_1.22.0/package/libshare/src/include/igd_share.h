#ifndef __IGD_SHARE__H__
#define __IGD_SHARE__H__
#ifndef __KERNEL__
#endif

#ifdef __KERNEL__
#include <linux/in.h>
#include <linux/netfilter_ipv4/nf_igd/product.h>
#else
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "product.h"
#endif

#ifndef ETH_ALEN
#define ETH_ALEN	6
#endif

#define UGRP_ALL	0
#define UGRP_WIFI	1
#define UGRP_WIRE	2
#define UGRP_AP_1	3 /* wire ap 1 */
#define UGRP_AP_2	4
#define UGRP_AP_3	5
#define UGRP_AP_4	6 
#define UGRP_PORT_1	7  /*  wire lan port 1 */
#define UGRP_PORT_2	8 
#define UGRP_PORT_3	9 
#define UGRP_PORT_4	10
#define UGRP_PORT_5	11
#define UGRP_5G_AP_1	12
#define UGRP_5G_AP_2	13
#define UGRP_5G_AP_3	14
#define UGRP_5G_AP_4	15
#define UGRP_SYS_MX	50

#define L7_MID_COMMON	0
#define L7_MID_GAME	1
#define L7_MID_HTTP	2
#define L7_MID_IM	3
#define L7_MID_STOCK	4
#define L7_MID_MOVIE	5
#define L7_MID_DL	6
#define L7_MID_READ	7
#define L7_MID_SHOP	8
#define L7_MID_OTHER	14

#define L7_MID_MX	15
#define L7_APP_MX	1000000
#define L7_GET_MID(appid) (appid / L7_APP_MX)
#define L7_GET_APPID(appid) (appid % L7_APP_MX)

enum {
	L7_APP_HTTP_DL = L7_APP_MX * L7_MID_DL + 1,
};
enum {
	L7_APP_HTTP = L7_APP_MX * L7_MID_HTTP,
};

enum {
	L7_APP_HTTP_MOVIE = L7_APP_MX * L7_MID_MOVIE + 65,
};
#define L7_APP_CHUYUN (L7_APP_MX * L7_MID_OTHER + 20)

enum {
	PPPD_TYPE_PPPOE_SERVER = 1,
	PPPD_TYPE_PPPOE_CLIENT,
};

enum {
	ISP_STAT_INIT = 0,
	ISP_STAT_AUTH_FAILD = 19,
	ISP_STAT_DNOT_REPLY,
	ISP_STAT_WISP_FINISH,
	ISP_STAT_IPCP_FINISH,
};

/*netlink broadcast grp id*/
enum {
	NLKMSG_GRP_SYS = 1,
	NLKMSG_GRP_HOST,
	NLKMSG_GRP_GROUP,
	NLKMSG_GRP_IF,
	NLKMSG_GRP_LOG,
	NLKMSG_GRP_L7,
};

enum {
	IF_GRP_MID_IPCGE = 1,
};

enum {
	LOG_GRP_MID_URL = 1,
	LOG_GRP_MID_UA,
	LOG_GRP_MID_HOST,
	LOG_GRP_MID_OS_TYPE,
};

enum {
	SYS_GRP_MID_WEB = 1,
	SYS_GRP_MID_DAY,
	SYS_GRP_MID_CONF,
	SYS_GRP_MID_DHCP,
	SYS_GRP_MID_ONLINE,
	SYS_GRP_MID_WIFI,
	SYS_GRP_MID_ADMIN,
};

/*end for netlink broadcast grp id*/

enum {	
	DNS_GRP = 0,
	URL_GRP,	
	USER_GRP,	
	GRP_MX,
};

#ifndef __KERNEL__
enum {
	NF_MOD_DNS = 0,
	NF_MOD_LOCAL,
	NF_MOD_NET,
	NF_MOD_QOS,
	NF_MOD_RATE,
	NF_MOD_MX,
};
#endif

#define sizelen(a) (sizeof(a) - 1)

#define IGD_FD_SET(fd, where) { FD_SET(fd, where); if (fd > max_fd) max_fd = fd; }
#define IGD_BITS_LONG (sizeof(long)*8)
#ifndef BITS_TO_LONGS
#define BITS_TO_LONGS(n) ((n + (sizeof(long)*8) - 1)/ (sizeof(long)* 8))
#endif
#define NRET_TRUE 0
#define NRET_FALSE -1

typedef unsigned long group_mask_t[BITS_TO_LONGS(GROUP_MX)];
typedef unsigned long rule_mask_t[BITS_TO_LONGS(RULE_MX)];
typedef uint32_t ip_t;
struct ip_range {
	ip_t start;
	ip_t end;
};
struct port_range {
	uint16_t start;
	uint16_t end;
};

struct pkt_stats {
	uint64_t bytes;
	unsigned long pkts;
	unsigned long speed;
};

struct flow_stats {
	struct pkt_stats now[2];
	struct pkt_stats last[2];
	unsigned long jiffies;
};

#define TYPE_ALL	0
#define TYPE_STD	1
#define TYPE_GID	2

#define INET_L3_NONE	0
#define INET_L3_STD	1
#define INET_L3_UID	2 /* doesn't support */
#define INET_L3_DNSID	3/* doesn't support */
#define INET_L3_DNS	4/* doesn't support */
#define INET_L3_URLID 5
#define INET_L3_URL 6
struct inet_l3 {
	int type;
	int gid; /* ip group id */
	struct ip_range addr; /* cpu bit order */
	char dns[64];
};

#define INET_HOST_NONE	0 /* match all */
#define INET_HOST_STD	1 /* match ip range */
#define INET_HOST_UGRP	2 /* match ugrp */
#define INET_HOST_MAC	3 /* match mac */
#define INET_HOST_PC	4 /* match pc */
#define INET_HOST_MOBILE 5 /* match MOBILE */

struct inet_host {
	int type;
	struct ip_range addr; /*  ip range */
	group_mask_t ugrp; /*  user group */
	unsigned char mac[6]; /* one mac */
};

#define INET_L4_NONE	0
#define INET_L4_DST	1
#define INET_L4_SRC	2
#define INET_L4_ALL	3 /* src+dst */
struct inet_l4 {
	int type;
	struct port_range src; /*  cpu bit order */
	struct port_range dst;/*  cpu bit order */
};

#define INET_L7_NONE	0
#define INET_L7_ALL	1
struct inet_l7 {
	int type;
	uint16_t mid;
	uint32_t appid;
};

#ifndef __KERNEL__
enum {
	URL_TARGET_PASS = 0,
	URL_TARGET_STUDY,
	URL_TARGET_REDIRECT,
	URL_TARGET_WEB_AUTH,
	URL_TARGET_DROP,
	URL_TARGET_ADVERT,
	URL_TARGET_MX,
};

enum {
	URL_MID_PASS = 0,
	URL_MID_WEB_AUTH,
	URL_MID_REDIRECT,
	URL_MID_DROP,
	URL_MID_ADVERT,
	URL_MID_MX,
};
#endif

#define NLK_ACTION_ADD 	0
#define NLK_ACTION_MOD	1
#define NLK_ACTION_DEL	2
#define NLK_ACTION_DUMP 3
#define NLK_ACTION_CLEAN 4

#define NLK_MSG_BUF_MAX	20480

struct nlk_msg_comm {
	int key;
	int action;
	int gid;
	int mid;
	int id;
	int obj_nr;
	int obj_len;
	int prio;
	int index;
};

/*for sys msg*/
struct sys_msg_ipcp {
	char devname[16];
	struct in_addr ip;
	struct in_addr mask;
	struct in_addr gw;
	struct in_addr dns[2];
	int type; /* lan or wan */
	int pppd_type;
	int uid; /*ui index */
};
struct sys_msg_wifi {
	int type;
	int stype;
	unsigned char mac[6];
};

struct sys_msg_admin {
	unsigned char mac[6];
};

union sys_msg_union {
	struct sys_msg_ipcp ipcp;
	struct sys_msg_wifi wifi;
	struct sys_msg_admin admin;
};

struct nlk_sys_msg {
	struct nlk_msg_comm comm;
	union sys_msg_union msg;
};
/*sys msg end*/

/*for dhcp info*/
struct host_dhcp_trait {
	unsigned char type;
	unsigned char op[32];
};

struct nlk_dhcp_msg {
	struct nlk_msg_comm comm;
	struct host_dhcp_trait dhcp[5];
};
/*dhcp info end*/

#define IGD_NAME_LEN 32
#define IGD_URL_HOST_LEN 32
#define IGD_URL_URI_LEN 32
#define IGD_SUFFIX_LEN 16
#define IGD_URL_LEN (IGD_URL_HOST_LEN + IGD_URL_URI_LEN)
#define IGD_DNS_LEN 32

enum {	IGD_ERR_FAILURE = -1,
	IGD_ERR_GROUP = -2, /* group invalid */
	IGD_ERR_NO_RESOURCE = -3, /*  resource erro */
	IGD_ERR_RULE_FULL = -4, /* rule full */
	IGD_ERR_NO_PERMISSION = -5, /* no permission */
	IGD_ERR_NO_MEM = -6, /* no memory */
	IGD_ERR_NAME_EXIST = - 7,
	IGD_ERR_NON_EXIST = -8, /* rule non exist */
	IGD_ERR_DATA_ERR = -9,
	IGD_ERR_EXIST = -10, /*  rule already exist */
	IGD_ERR_NOT_SUPPORT = -11,
};

enum {
	USER_GRP_TYPE_MAC,
	USER_GRP_TYPE_IP,
};

struct inet_url {
	char url[IGD_URL_LEN];
	uint32_t id;
};

struct nlk_dump_rule{
	int id;
	char name[IGD_NAME_LEN];
};

struct nlk_grp_rule {
	struct nlk_msg_comm comm;
	char name[IGD_NAME_LEN];
	int type;
};

/*use for filter module*/

enum {
	NF_TARGET_ACCEPT = 0,
	NF_TARGET_DENY,
	NF_TARGET_REPLACE,
	NF_TARGET_REDIRECT,
	NF_TARGET_QOS,
	NF_TARGET_RATE,
};

struct nf_rule_target_redirect {
	uint32_t ip;
	unsigned char url[IGD_URL_LEN];
};

struct nf_rule_target_replace {
	uint32_t org_ip;
	uint32_t rep_ip;
};
struct nf_rule_target_rate {
	uint16_t up; /* kbytes per seconds */
	uint16_t down;/* kbytes per seconds */
	uint16_t up_share; /*  doesn't support */
	uint16_t down_share; /* doesn't support */
};
struct nf_rule_target_qos {
	uint32_t queue; /*queue id*/
};

struct nf_rule_target_accept {
};

struct nf_rule_target_deny {
};

struct nf_rule_target {
	uint32_t t_type;//drop, accept, redirect, replace
	union {
		struct nf_rule_target_accept accept;
		struct nf_rule_target_deny deny;
		struct nf_rule_target_redirect redirect;
		struct nf_rule_target_replace replace;
		struct nf_rule_target_rate rate;
		struct nf_rule_target_qos qos;
	} target;
};

#define INET_PROTO_ALL 0
#define INET_RPOTO_T_U 255
struct nf_rule_base {
	struct inet_host host;
	struct inet_l3 l3;
	struct inet_l4 l4;
	struct inet_l7 l7; /* doesn't use, abosolated   */
	uint8_t protocol;
};

struct nf_rule_info {
	struct nf_rule_base base;
	struct nf_rule_target target;
};

struct nf_rule_u {
	struct nlk_msg_comm comm;
	struct nf_rule_base base;
	struct nf_rule_target target;
	unsigned long bitmap; /* used by l7 */
	int extra;
};
/*end for filter module*/
#define NLK_HW_FLASH	0
#define NLK_HW_LED	1
#define NLK_HW_MII	2
#define NLK_NETDEV_VLAN	0
#define NLK_NETDEV_UID	1

enum NLK_MSG_TYPE {
	NLK_MSG_START = 1, //must first
	NLK_MSG_RET,       //kernel to user msg type
	NLK_MSG_GET_STATISTICS,
	NLK_MSG_L7,
	NLK_MSG_GROUP,
	NLK_MSG_HOST,
	NLK_MSG_RULE_ACTION,
	NLK_MSG_URL_SAFE,
	NLK_MSG_URL_LOG,
	NLK_MSG_WEB_AUTH,
	NLK_MSG_DAY, /*day change */
	NLK_MSG_UA,
	NLK_MSG_HW,
	NLK_MSG_BROADCAST,
	NLK_MSG_URL, /* url pass and drop rule */
	NLK_MSG_NETDEV,
	NLK_MSG_END = 0x7FFF, //must  last
};

/*used for url safe*/
#define DNS_IP_MX 2
enum url_safe_msg {
	URL_SET_PARAM = 1,
	URL_SET_SAFE_IP,
	URL_SEC_ENABLE,
	URL_SEC_WLIST,
};

struct nlk_url_set {
	struct nlk_msg_comm comm;
	uint32_t enable;
};

struct nlk_url_ip {
	struct nlk_msg_comm comm;
	uint32_t addr;
	unsigned char url[IGD_URL_HOST_LEN];
};

struct nlk_url_sec {
	struct nlk_msg_comm comm;
	uint32_t devid;
	uint32_t sip[DNS_IP_MX];//server ip
	uint32_t rip;//redirect ip
	uint16_t sport;//server port
	uint16_t rport;//redirect port
	char rdhost[IGD_URL_HOST_LEN];
};
/*url safe end*/

/*url statistics start*/

#define URL_LOG_LIST_MX 256
#define URL_LOG_MX_NR 128
#define URL_LOG_URL_LEN 150

enum {
	NLK_URL_LOG_PARAM,
	NLK_URL_LOG_DUMP,
};

struct nlk_url_data {
	uint16_t type;
	uint16_t id;
	char suffix[IGD_SUFFIX_LEN];
	char url[IGD_URL_HOST_LEN];
	char uri[IGD_URL_URI_LEN];
};

struct nlk_url_log {
	uint16_t type;
	uint16_t id;
	time_t time;
	unsigned char mac[6];
	char url[URL_LOG_URL_LEN];
};

struct nlk_url {
	struct nlk_msg_comm comm;
	struct inet_host src;
	struct inet_l3 l3; /* only support url and url id */
	int times;/*match times*/
	int type;
};

/*url statistics end*/

/*web auth start*/
enum {
	WEB_AUTH_SET_PARAM = 0,
	WEB_AUTH_SET_HOST_MAC,
	WEB_AUTH_GET_HOST_STA,
};

struct nlk_web_auth {
	struct nlk_msg_comm comm;
	int enable;
	group_mask_t grp;
	unsigned char mac[ETH_ALEN];
};
/*web auth end*/

struct nlk_host {
	struct nlk_msg_comm comm;
	struct in_addr addr;
	unsigned char mac[6];
	struct pkt_stats pkt[2];
};

#define NLK_HTTP_HOST_SHOP	0
#define NLK_HTTP_HOST_HOST	1
#define NLK_HTTP_HOST_URL	2

struct nlk_http_host {
	struct nlk_msg_comm comm;
	unsigned char mac[6];
};

struct http_host_log {
	char host[IGD_URL_HOST_LEN];
	char uri[IGD_NAME_LEN];
	unsigned long seconds;
	int times;
};

struct nlk_msg_l7 {
	struct nlk_msg_comm comm;
	struct in_addr addr;
	unsigned char mac[ETH_ALEN];
	int mid;
	int appid;
	struct pkt_stats pkt[2];
};

struct nlk_netdev {
	struct nlk_msg_comm comm;
	char devname[16];
	uint16_t uid;
	uint16_t vlan_mask;
};

#define HTTP_RULE_KEY_MX	8

struct nlk_http_key {
	unsigned char key[20];
	uint8_t dir; /* dir */
	uint8_t type; /* match type*/
	uint8_t len; /*len of value*/
	int8_t offset; /* offset  */
	uint8_t end; /*end of str*/
	unsigned char value[19];
};

enum {
	L7_NLK_SET,
	L7_NLK_RULE,
	L7_NLK_HTTP,
	L7_NLK_GATHER,
};

struct if_pkt_stats {
	struct pkt_stats all;
	struct pkt_stats udp;
	struct pkt_stats tcp;
	struct pkt_stats icmp;
};

struct if_statistics {
	struct if_pkt_stats in;
	struct if_pkt_stats out;
};

enum SYS_FLAG_BIT {
	SYSFLAG_RESET = 0,
	SYSFLAG_RECOVER,
	SYSFLAG_ALIRESET,
	SYSFLAG_MAX,//must last
};

enum HOST_OS_TYPE {
	OS_TYPE_NONE = 0,
	OS_PC_WINDOWS,
	OS_PC_LINUX,
	OS_PC_MACOS,
	OS_PHONE_ANDROID,
	OS_PHONE_IOS,
	OS_PHONE_WP,
	OS_TABLET_IOS,
	OS_TABLET_ANDROID,
	OS_TABLET_SURFACE,
};

#define CONN_DUMP_MX	1000
#define NLK_HOST		0
#define NLK_HOST_CONN		1
#define NLK_HOST_APP		2

struct conn_info {
	struct in_addr sip;
	struct in_addr dip;
	uint16_t sport;
	uint16_t dport;
	uint16_t mid;
	uint16_t appid;
	uint8_t proto;
	unsigned long seconds;
	struct pkt_stats send;
	struct pkt_stats rcv;
};

struct app_conn_info {
	uint16_t mid;
	uint16_t conn_cnt;
	uint32_t appid;
	unsigned long seconds;
	struct pkt_stats send;
	struct pkt_stats rcv;
};

struct host_info {
	struct in_addr addr;
	unsigned char mac[6];
	struct pkt_stats send; /* send pkts */
	struct pkt_stats rcv; /*  rcv pkts */
	char name[32]; /* host name */
	char nick_name[32]; /* nick_name set by user*/
	int conn_cnt; /*  conn cnt */
	int seconds; /*  online seconds */
	uint16_t vender;
	uint8_t os_type;
	uint8_t is_wifi; /* set 1 if wifi */
	uint8_t pid; /* port id:wire 1-4, wifi: 1-4*/
};

#define UA_DATA_MX 1024
struct ua_data {
	uint8_t skip;
	uint8_t len;
	uint8_t end;
	uint8_t start_len;
	uint8_t end_len;
	uint8_t name_len;
	uint16_t vender;
	char match_start[32];
	char match_end[16];
	char host_name[16];
};

struct ua_msg {
	struct nlk_msg_comm comm;
	unsigned char mac[ETH_ALEN];
	char ua[128];
	int vender;
};

struct rate_entry {
	uint16_t up;
	uint16_t down;
	uint16_t share_up; /*share rate, doesn't support*/
	uint16_t share_down;  /* doesn't support */
};

#define APP_RULE_MAX 2048
struct app_filter_rule {
	int type;
	int prio;
	int nr;
	int appid[APP_RULE_MAX];
};

struct time_comm {
	unsigned char loop;
	unsigned char day_flags; //6 5 4 3 2 1 0
	unsigned char start_hour;
	unsigned char start_min;
	unsigned char end_hour;
	unsigned char end_min;
};

#define RD_KEY_IP	0
#define RD_KEY_MAC	1
#define RD_KEY_RIP	2
#define RD_KEY_RMAC	3
#define RD_KEY_SSID	4
#define RD_KEY_URL	29

struct url_rd {
	unsigned long flags;
	char args[128];
	char url[128];
};

/*  array copy */
#define arr_strcpy(dst,src) do{strncpy(dst,src,sizeof(dst)-1);dst[sizeof(dst)-1]=0;}while(0)
#define arr_strcpy_end(dst,src,end) __arr_strcpy_end(dst,src,sizeof(dst)-1,end)
#define igd_min(a,b) (a > b ? a:b)
#define loop_for_each(min, mx) do {int i; for (i = min; i < mx; i++)
#define loop_for_each_2(val, min, max) do{int val; for (val = min; val < (max); val++)
#define loop_for_each_3(val, min, max) do{for (val = min; val < (max); val++) 
#define loop_for_each_end() }while(0)
#define MAC_SPLIT(mac) mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]
#define MAC_FORMART "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx"

static inline int igd_test_bit(int nr, unsigned long *bit)
{
	unsigned long mask = 1UL << (nr % IGD_BITS_LONG);
	unsigned long *p = bit + nr / IGD_BITS_LONG;

	return (*p & mask) != 0;
}

static inline int igd_set_bit(int nr, unsigned long *bit)
{
	unsigned long mask = 1UL << (nr % IGD_BITS_LONG);
	unsigned long *p = bit + nr / IGD_BITS_LONG;
	*p |= mask;
	return 0;
}

static inline int igd_clear_bit(int nr, unsigned long *bit)
{
	unsigned long mask = 1UL << (nr % IGD_BITS_LONG);
	unsigned long *p = bit + nr / IGD_BITS_LONG;
	*p &= ~mask;
	return 0;
}

static inline unsigned char *strstr_end(unsigned char *src,
	       		int len, unsigned char *key, int klen, int end)
{
	unsigned char *tmp = src;

	while (len >= klen) {
		if (!*tmp || *tmp == end)
			break;
		if (!memcmp(tmp, key, klen)) 
			return tmp;
		tmp++;
		len--;
	}
	return NULL;
}

static inline unsigned char *strstr_none(unsigned char *str,
					int len, unsigned char *key, int klen)
{
	unsigned char *tmp = str;
	
	while (len >= klen) {
		if (!memcmp(tmp, key, klen))
			return tmp;
		tmp++;
		len--;
	}
	return NULL;
}

static inline uint32_t __arr_strcpy_end(unsigned char *dst,
	       	unsigned char *src, int len, int end)
{
	uint32_t i = 0;
	while (src[i] && i < len) {
		if (src[i] == end)
			break;
		dst[i] = src[i];
		i++;
	}
	dst[i] = 0;
	return i;
}

static inline void parse_mac(const char *macaddr, unsigned char mac[6])
{
	unsigned int m[6];
	if (sscanf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
		   &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]) != 6)
		return;
	mac[0] = m[0];
	mac[1] = m[1];
	mac[2] = m[2];
	mac[3] = m[3];
	mac[4] = m[4];
	mac[5] = m[5];
}
#endif

#ifndef _IGD_MU_INTERFACE_
#define _IGD_MU_INTERFACE_

enum {
	IF_TYPE_LAN = 1,
	IF_TYPE_WAN,	
};

enum {
	MODE_DHCP = 1,
	MODE_PPPOE,
	MODE_STATIC,
	MODE_WISP,
};

enum {
	IF_STATUS_INIT = 1,
	IF_STATUS_IP_UP,
	IF_STATUS_IP_DOWN,
};

enum {
	NET_OFFLINE = 0,
	NET_ONLINE,
};

enum {
	DETECT_OFF = 0,
	DETECT_ON,
};

#define WAN_BAK "wanbak"
#define DNS_DOMAIN "www.baidu.com"
#define IF_CONFIG_FILE "network"
#define WAN_DNS_FILE "/var/resolv.conf.auto"
#define WAN_DHCP_PID "/var/run/udhcpc.pid"

#define PPPOE_SERVER_CMD "pppoe-server -I br-lan -k -F -L 10.254.254.1 &"

#define PPPOE_MODE_FILE "/tmp/pppoe_dhcp"
#define PPPOE_DETECT_PID "/var/run/pppoe_dhcp.pid"
#define PPPOE_DETECT_CMD "/usr/bin/pppoe_dhcp -I eth0.2 -t 1 -F 1 -p /var/run/pppoe_dhcp.pid &"

#define NETDETECT "netdetect"

#define IF_MOD_INIT DEFINE_MSG(MODULE_INTERFACE, 0)
enum {
	IF_MOD_PARAM_SET = DEFINE_MSG(MODULE_INTERFACE, 1),
	IF_MOD_PARAM_SHOW,
	IF_MOD_IFACE_INFO,
	IF_MOD_IFACE_IP_UP,
	IF_MOD_IFACE_IP_DOWN,
	IF_MOD_WAN_DETECT,
	IF_MOD_GET_ACCOUNT,
	IF_MOD_SEND_ACCOUNT,
	IF_MOD_NET_STATUS,
	IF_MOD_DETECT_REPLY,
	IF_MOD_ISP_STATUS,
	IF_MOD_PARAM_BAK,
};

struct detect_info {
	int link;
	int net;
	int mode;
};

struct account_info {	
	char user[IGD_NAME_LEN];
	char passwd[IGD_NAME_LEN];
	uint8_t peermac[ETH_ALEN];
};

struct pppoe_conf {
	char user[IGD_NAME_LEN];
	char passwd[IGD_NAME_LEN];
};

struct static_conf {
	struct in_addr ip;	
	struct in_addr mask;	
	struct in_addr gw;	
};

struct wisp_conf {
	int channel;
	char ssid[IGD_NAME_LEN];
	char key[IGD_NAME_LEN];
	char security[IGD_NAME_LEN];
};

struct mode_conf {
	int mtu;
	struct in_addr dns[DNS_IP_MX];
	uint8_t mac[ETH_ALEN];
	struct pppoe_conf pppoe;
	struct static_conf statip;
	struct wisp_conf wisp;
};

struct iface_conf {
	int uid;
	int mode;
	struct mode_conf isp;
};

struct iface_info {
	int type;
	int uid;
	int mode;
	int link; //link up or link down
	int net; //is connected to internet
	int err;
	char devname[IGD_NAME_LEN];
	char uiname[IGD_NAME_LEN];
	uint8_t mac[ETH_ALEN];
	int mtu;
	struct in_addr ip;
	struct in_addr mask;
	struct in_addr gw;
	struct in_addr dns[DNS_IP_MX];
};

struct iface;
struct iface_hander {
	int (*if_up)(struct iface *ifc, struct iface_conf *config);
	int (*if_down)(struct iface *ifc);
	int (*ip_up)(struct iface *ifc, struct sys_msg_ipcp *ipcp);
	int (*ip_down)(struct iface *ifc, struct sys_msg_ipcp *ipcp);
};

struct iface {
	int type;
	int uid;
	char devname[IGD_NAME_LEN];
	char uiname[IGD_NAME_LEN];
	unsigned long status;
	struct iface_hander *hander;
	struct iface_conf config;
	struct iface_conf confbak;
	struct sys_msg_ipcp ipcp;
};

extern int interface_call(MSG_ID msgid, void *data, int len, void *rbuf, int rlen);
#endif


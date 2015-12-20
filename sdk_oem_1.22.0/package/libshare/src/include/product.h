#ifndef __NETFILTER_NF_IGD_PRODUCT__H__
#define __NETFILTER_NF_IGD_PRODUCT__H__

#define RULE_MX 513
#define GROUP_MX 256
#define IGD_URL_GRP_PER_MX 1536
#define IGD_USER_GRP_PER_MX 128
#define IGD_DNS_GRP_PER_MX 128

#define NF_WAN_NUM_MX 1
#define NF_LAN_NUM_MX 1
#define NF_PORT_NUM_MX (NF_WAN_NUM_MX + NF_LAN_NUM_MX)
#define WIFI_DEVNAME "ra0"
#define VAP_DEVNAME "ra1"
#define APCLI_DEVNAME "apcli0"
#define LAN_DEVNAME "br-lan"
#define LAN_UINAME "lan"
#define WIRE_DEVNAME	"eth0.1"
#define WAN1_DEVNAME "eth0.2"
#define WAN1_UINAME "wan"
#define WAN1_VIRNAME "vir1"
#define WIFI_NAME	"ra"
#define WIFI_5G_NAME	"rai"
#define WIFI_NAME_LEN	2
#define WIFI_5G_NAME_LEN	3
#define WIFI_5G_BASIC_ID	10
#define PORT_MX		5

#define HOST_MX		64
#define HOST_ALL_MX	64
#define CONN_MX		4000
#define HTTP_HOST_LOG_PER_MX	256
#endif

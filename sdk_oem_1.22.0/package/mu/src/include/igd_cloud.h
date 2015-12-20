#ifndef __CLOUD_COMM_H__
#define __CLOUD_COMM_H__

#include "module.h"

/*
CSO: cloud server order
*/
#define CSO_NTF_KEY 1000
#define CSO_REQ_ROUTER_FIRST 1002
#define CSO_ACK_ROUTER_FIRST 1003
#define CSO_REQ_ROUTER_AGAIN 1004
#define CSO_ACK_ROUTER_AGAIN 1005
#define CSO_REQ_ROUTER_LOGIN 1006
#define CSO_ACK_ROUTER_LOGIN 1007
#define CSO_NTF_ROUTER_REDIRECT 1008
#define CSO_REQ_ROUTER_KEEP 1101
#define CSO_ACK_ROUTER_KEEP 1102
#define CSO_NTF_ROUTER_TRANSFER 1103
#define CSO_NTF_ROUTER_VERSION 1104
#define CSO_REQ_ROUTER_RELEASE 1105
#define CSO_ACK_ROUTER_RELEASE 1106
#define CSO_NTF_ROUTER_HEARWARE 1107
#define CSO_NTF_ROUTER_BIND 1108
#define CSO_NTF_ROUTER_RELEASE 1109

#define CSO_NTF_ROUTER_FLOW  1110
#define CSO_NTF_ROUTER_TERMINAL  1111
#define CSO_NTF_ROUTER_APP  1112
#define CSO_REQ_ROUTER_RECORD  1113
#define CSO_ACK_ROUTER_RECORD  1114
#define CSO_REQ_CONNECT_ONLINE  1115
#define CSO_ACK_CONNECT_ONLINE  1116
#define CSO_NTF_ROUTER_MMODEL  1117
#define CSO_NTF_ROUTER_MAC  1118
#define CSO_REQ_ROUTER_RESET  1119
#define CSO_ACK_ROUTER_RESET  1120
#define CSO_REQ_SITE_CUSTOM  1121
#define CSO_ACK_SITE_CUSTOM  1122
#define CSO_REQ_SITE_RECOMMAND  1123
#define CSO_ACK_SITE_RECOMMAND  1124
#define CSO_REQ_VISITOR_MAC  1125
#define CSO_ACK_VISITOR_MAC  1126
#define CSO_NTF_GAME_TIME  1127
#define CSO_NTF_DEL_HISTORY  1128
#define CSO_NTF_ROUTER_APP_ONOFF  1129
#define CSO_NTF_ROUTER_ONLINE  1130
#define CSO_REQ_ROUTER_SSID  1131
#define CSO_ACK_ROUTER_SSID  1132
#define CSO_NTF_ROUTER_UPDOWN  1133
#define CSO_NTF_ROUTER_CODE  1134
#define CSO_NTF_ROUTER_PRICE  1135
#define CSO_ACK_ROUTER_PRICE  1136
#define CSO_REQ_ROUTER_CONFIG  1137
#define CSO_ACK_ROUTER_CONFIG  1138
#define CSO_REQ_CONFIG_VER	1139
#define CSO_ACK_CONFIG_VER  1140

#define IGD_CLOUD_CACHE_MX 100
#define IGD_CLOUD_MSG_LEN 0x2000
#define CLOUD_RCVBUF_SIZE_MAX   (0x4000)

#define CC_PUSH1(buf,i,d)   (*((unsigned char *)&buf[i]) = (unsigned char)(d))
#define CC_PUSH2(buf,i,d)   (*((unsigned short *)&buf[i]) = htons(d))
#define CC_PUSH4(buf,i,d)   (*((unsigned int *)&buf[i]) = htonl(d))
#define CC_PUSH_LEN(buf,i,data,len)   memcpy(buf+(i),data,len)

//used by mu progress
#define CC_CALL_ADD(sbuf,slen)  mu_call(IGD_CLOUD_CACHE_ADD, sbuf, slen, NULL, 0)
#define CC_CALL_DUMP(rbuf,rlen)  mu_call(IGD_CLOUD_CACHE_DUMP, NULL, 0, rbuf, rlen)

//used by other progress
#define CC_MSG_ADD(sbuf,slen)  mu_msg(IGD_CLOUD_CACHE_ADD, sbuf, slen, NULL, 0)
#define CC_MSG_DUMP(rbuf,rlen)  mu_msg(IGD_CLOUD_CACHE_DUMP, NULL, 0, rbuf, rlen)

//used by mu progress, fail: <=0, success: >0, return strlen(value)
#define CC_CALL_RCONF(flag,value,value_len) \
	mu_call(IGD_CLOUD_RCONF_GET, flag, strlen(flag), value, value_len)

//used by other progress, fail: <=0, success: >0, return strlen(value)
#define CC_MSG_RCONF(flag,value,value_len) \
	mu_msg(IGD_CLOUD_RCONF_GET, flag, strlen(flag) + 1, value, value_len)

//used by mu progress, fail: < 0, success: 0
#define CC_CALL_RCONF_INFO(flag,value,value_len) \
	mu_call(IGD_CLOUD_RCONF_GET_INFO, flag, strlen(flag), value, value_len)

//used by other progress, fail: < 0, success: 0
#define CC_MSG_RCONF_INFO(flag,value,value_len) \
	mu_msg(IGD_CLOUD_RCONF_GET_INFO, flag, strlen(flag) + 1, value, value_len)

/* CCT: cloud config type */
enum CLOUD_CONF_TYPE {
	CCT_L7 = 0,
	CCT_UA,
	CCT_DOMAIN,
	CCT_WHITE,
	CCT_VENDER,
	CCT_TSPEED,
	CCT_STUDY_URL,
	CCT_JS,
	CCT_ERRLOG, //must together be first
	CCT_URLLOG, //must together
	CCT_URLSAFE, //must together
	CCT_URLSTUDY, //must together
	CCT_URLHOST, //must together
	CCT_URLREDIRECT, //must together
	CCT_CHECKUP, //must together be last
	CCT_MAX,
};

#define RCONF_RETRY_NUM   3
#define RCONF_CHECK   "/etc/rconf_check"
#define RCONF_CHECK_TMP   "/tmp/rconf_check"
#define RCONF_DIR  "/etc/rconf"
#define RCONF_DIR_TMP  "/tmp/rconf"
#define RCONF_DIR_NEW  "/tmp/rconf_new"

#define RCONF_FLAG_VER           "VER:"
#define RCONF_FLAG_L7            "F01:"
#define RCONF_FLAG_UA            "F02:"
#define RCONF_FLAG_DOMAIN        "F03:"
#define RCONF_FLAG_WHITE         "F04:"
#define RCONF_FLAG_VENDER        "F05:"
#define RCONF_FLAG_TSPEED        "F06:"
#define RCONF_FLAG_STUDY_URL     "F07:"
#define RCONF_FLAG_JS            "F08:"
#define RCONF_FLAG_ERRLOG        "F09:"
#define RCONF_FLAG_URLLOG        "F10:"
#define RCONF_FLAG_URLSAFE       "F11:"
#define RCONF_FLAG_URLSTUDY      "F12:"
#define RCONF_FLAG_URLHOST       "F13:"
#define RCONF_FLAG_URLREDIRECT   "F14:"
#define RCONF_FLAG_CHECKUP       "F15:"

struct nlk_cloud_config {
	struct nlk_msg_comm comm;
	int ver[CCT_MAX];
};

enum {
	IGD_CLOUD_INIT = DEFINE_MSG(MODULE_CLOUD, 0),
	IGD_CLOUD_CACHE_ADD,
	IGD_CLOUD_CACHE_DUMP,
	IGD_CLOUD_RCONF,
	IGD_CLOUD_RCONF_FLAG,
	IGD_CLOUD_RCONF_VER,
	IGD_CLOUD_RCONF_GET,
	IGD_CLOUD_RCONF_GET_INFO,
};

extern int igd_cloud_call(MSG_ID msgid, void *data, int d_len, void *back, int b_len);
#endif //__CLOUD_COMM_H__

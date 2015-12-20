#ifndef __IGD_WIFI_H_
#define __IGD_WIFI_H_

#define WIFI_CONFIG_FILE "wireless"
#define WIFI_DATA_FILE "/etc/Wireless/mt7620/mt7620.dat"
#define WIFI_DEV "mt7620"

struct wifi_conf {
	int enable;
	int channel;
	int txrate;
	int vap;
	int hidssid;
	int hidvssid;
	int htbw;
	int mode;
	int time_on;
	struct time_comm tm;
	char ssid[IGD_NAME_LEN];
	char vssid[IGD_NAME_LEN];
	char encryption[IGD_NAME_LEN];
	char key[IGD_NAME_LEN];
};

struct iwsurvey {
	unsigned char ch;
	unsigned char signal;
	unsigned char wps;
	char extch[IGD_NAME_LEN];
	char security[IGD_NAME_LEN];
	char mode[IGD_NAME_LEN];
	char bssid[IGD_NAME_LEN];
	char ssid[IGD_NAME_LEN];
};

#define IW_CMD_MX 32
#define IW_SUR_MX 32
#define IW_LIST_MX 32

struct acl_entry {
	unsigned char mac[6];
	unsigned short enable;
};

struct acl_list {
	int nr;
	int enable;
	struct acl_entry list[IW_LIST_MX];
};

struct iwacl {
	struct acl_list wlist;
	struct acl_list blist;
};

enum acl_mode {
	BLACK_ACL = 0,
	WHITE_ACL,
};

#define WIFI_MOD_INIT DEFINE_MSG(MODULE_WIFI, 0)
enum {
	WIFI_MOD_SET_AP = DEFINE_MSG(MODULE_WIFI, 1),
	WIFI_MOD_SET_VAP,
	WIFI_MOD_SET_TIME,
	WIFI_MOD_GET_CONF,
	WIFI_MOD_SET_CHANNEL,
	WIFI_MOD_SET_TXRATE,
	WIFI_MOD_GET_CHANNEL,
	WIFI_MOD_GET_SURVEY,
	WIFI_MOD_VAP_HOST_ADD,
	WIFI_MOD_VAP_HOST_DEL,
	WIFI_MOD_VAP_HOST_DUMP,
	WIFI_MOD_VAP_HOST_ONLINE,
	WIFI_MOD_DISCONNCT_ALL,
	WIFI_UPLOAD_SUCCESS,
	WIFI_MOD_SET_MODE,
	WIFI_MOD_SET_ACL,
	WIFI_MOD_GET_ACL,
	WIFI_MOD_SET_HIDDEN,
};

extern int wifi_call(MSG_ID msgid, void *data, int len, void *rbuf, int rlen);
#endif
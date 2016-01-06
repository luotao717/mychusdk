#ifndef __IGD_WIFI_H_
#define __IGD_WIFI_H_

#define WIFI_CONFIG_FILE "wireless"

enum wmode {
	MODE_802_11_BG = 0,
	MODE_802_11_B,
	MODE_802_11_A,
	MODE_802_11_ABG,
	MODE_802_11_G,
	MODE_802_11_ABGN,
	MODE_802_11_N,
	MODE_802_11_GN,
	MODE_802_11_AN,
	MODE_802_11_BGN,
	MODE_802_11_AGN,
	MODE_802_11_5N,
	MODE_802_11_AANAC = 14,
	MODE_802_11_ANAC,
};

struct wifi_conf {
	int valid;
	int enable;
	int channel;
	int txrate;
	int vap;
	int hidssid;
	int hidvssid;
	int htbw;
	int mode;
	int ifindex;
	int apindex;
	int vapindex;
	int time_on;
	struct time_comm tm;
	char ssid[IGD_NAME_LEN];
	char vssid[IGD_NAME_LEN];
	char encryption[IGD_NAME_LEN];
	char key[IGD_NAME_LEN];
	char apname[IGD_NAME_LEN];
	char vapname[IGD_NAME_LEN];
	char devname[IGD_NAME_LEN];
	char apcname[IGD_NAME_LEN];
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

struct iwguest {
	int gid;
	unsigned char mac[ETH_ALEN];
};

#define IW_CMD_MX 32
#define IW_SUR_MX 32
#define IW_LIST_MX 32
#define IW_IF_MX 2

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
	int ifindex;
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
	WIFI_MOD_DUMP_STATUS,
	WIFI_MOD_SET_MODE,
	WIFI_MOD_SET_ACL,
	WIFI_MOD_GET_ACL,
	WIFI_MOD_GET_CONF_5G,
	WIFI_MOD_SET_CHANNEL_5G,
	WIFI_MOD_SET_TXRATE_5G,
	WIFI_MOD_GET_CHANNEL_5G,
	WIFI_MOD_GET_SURVEY_5G,
	WIFI_MOD_SET_MODE_5G,
	WIFI_MOD_SET_ACL_5G,
	WIFI_MOD_GET_ACL_5G,
};

extern int wifi_call(MSG_ID msgid, void *data, int len, void *rbuf, int rlen);
#endif
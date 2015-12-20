#ifndef _IGD_ADVERT_
#define _IGD_ADVERT_

#define IGD_ADVERT "igd_advert"
#define ADVERT_MOD_INIT DEFINE_MSG(MODULE_ADVERT, 0)

enum {
	ADVERT_MOD_UPDATE = DEFINE_MSG(MODULE_ADVERT, 1),
	ADVERT_MOD_ACTION,
	ADVERT_MOD_DUMP,
	ADVERT_MOD_UPLOAD_SUCCESS,
};

struct advert_rule {
	int nr;
	int gid;
	int rid;
	struct list_head list;
	struct inet_host host;
	struct http_rule_api args;
	char url[IGD_URL_GRP_PER_MX][IGD_URL_LEN];
};

#define ADVERT_CONFIG "qos_rule"
#define ADVERT_FILE "/tmp/rconf/advert.txt"

extern int advert_call(MSG_ID msgid, void *data, int len, void *rbuf, int rlen);
#endif


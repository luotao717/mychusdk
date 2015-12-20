#ifndef _ALI_CLOUD_
#define _ALI_CLOUD_

#define ALI_CLOUD "ali_cloud"
#define ALI_INIT_FILE "/etc/init.d/ali_cloud.init"
#define ALI_MOD_INIT DEFINE_MSG(MODULE_ALI, 0)

enum {
	ALI_MOD_RESTART = DEFINE_MSG(MODULE_ALI, 1),
};

extern int ali_call(MSG_ID msgid, void *data, int len, void *rbuf, int rlen);
#endif
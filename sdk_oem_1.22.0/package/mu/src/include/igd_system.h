#ifndef _IGD_MODULE_SYSTEM_
#define _IGD_MODULE_SYSTEM_

struct fw_info {
	int status;
	int flag;
	int cur;
	int speed;
	char size[16];
	char url[256];
	char md5[64];
	char ver[64];
	char cur_ver[64];
	char name[64];
	char fw_file[64];
	char time[64];
	char desc[1024];
};

enum {
	FLAG_LOCAL = 0,
	FLAG_SERVER,
	FLAG_CLOUD,
};

enum {
	FW_STATUS_INIT = 0,
	FW_STATUS_CHECKING,
	FW_STATUS_CHCFINISH,
	FW_STATUS_DOWNLOAD,
};

enum {
	FW_FLAG_NOFW = 0,
	FW_FLAG_HAVEFW,
	FW_FLAG_DOWNING,
	FW_FLAG_SUCCESS,
	FW_FLAG_FAILE,
	FW_FLAG_FINISH,
};

struct sys_account {
	char user[16];
	char password[16];
};

#define SYS_CONFIG_FILE "qos_rule"

#define FW_VER_KEY "CURVER"
#define FW_VENDOR_KEY "VENDOR"
#define FW_PRODUCT_KEY "PRODUCT"
#define FW_INFO_FILE "/etc/firmware"
#define FW_STATUS_FILE "/tmp/fwdir/wgetend"
#define FW_FILE  "/tmp/fwdir/firmware.bin"
#define FW_LOCAL_FILE  "/tmp/ioos/firmware.bin"
#define FW_CLOUD_FILE  "/tmp/firmware.bin"

#define FW_SERVER_PORT "80" 
#define FW_SERVER_DOMAIN "api.wiair.com"
		
#define SYSTEM_MOD_INIT DEFINE_MSG(MODULE_SYSTEM, 0)
enum {
	SYSTEM_MOD_DEV_CHECK = DEFINE_MSG(MODULE_SYSTEM, 1),
	SYSTEM_MOD_FW_UPDATE,
	SYSTEM_MOD_VERSION_CHECK,
	SYSTEM_MOD_VERSION_INFO,
	SYSTEM_MOD_FW_DOWNLOAD,
	SYSTEM_MOD_SYS_REBOOT,
	SYSTEM_MOD_SYS_DEFAULT,
	SYSTME_MOD_LOCAL_UPDATE,	
	SYSTME_MOD_CLOUD_UPDATE,
	SYSTME_MOD_SET_LED,
	SYSTME_MOD_GET_ACCOUNT,
	SYSTME_MOD_SET_ACCOUNT,
	SYSTME_MOD_SYS_CMD,
	SYSTME_MOD_MTD_PARAM,
};

extern int system_call(MSG_ID msgid, void *data, int len, void *rbuf, int rlen);
#endif

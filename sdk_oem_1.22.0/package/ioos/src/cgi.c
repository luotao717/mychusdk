#include "protocol.h"
#include "server.h"
#include <linux/if.h>
#include "nlk_ipc.h"
#include "ioos_uci.h"
#include "log.h"
#include "igd_host.h"
#include "igd_lib.h"
#include "igd_interface.h"
#include "igd_wifi.h"
#include "igd_dnsmasq.h"
#include "igd_qos.h"
#include "igd_system.h"
#include "igd_url_safe.h"
#include "igd_upnp.h"
#include "igd_nat.h"
#include "igd_cloud.h"
#include "igd_advert.h"
#include "uci_fn.h"
#include "igd_md5.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "image.h"

#if 0
#define CGI_PRINTF(fmt,args...) do{}while(0)
#else
#define CGI_PRINTF(fmt,args...) do{console_printf("[CGI:%05d]DBG:"fmt, __LINE__, ##args);}while(0)
#endif

#define CGI_BIT_SET(d,f)  (d) |= (1<<(f))
#define CGI_BIT_CLR(d,f)  (d) &= (~(1<<(f)))
#define CGI_BIT_TEST(d,f)  ((d) & (1<<(f)))

enum {
	CGI_ERR_FAIL = 10001,
	CGI_ERR_INPUT,
	CGI_ERR_MALLOC,
	CGI_ERR_EXIST,
	CGI_ERR_NONEXIST,
	CGI_ERR_FULL,
	CGI_ERR_NOLOGIN,
	CGI_ERR_NOSUPPORT,
	CGI_ERR_ACCOUNT_NOTREADY,
	CGI_ERR_TIMEOUT,
	CGI_ERR_FILE,
};

#define CHECK_LOGIN do {\
	if(con->login != 1) {\
		CGI_PRINTF("WARNING: NO LOGIN\n");\
		return CGI_ERR_NOLOGIN;\
	}\
} while (0)

char *cgi_mac2str(uint8_t *mac)
{
	static char str[20];

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%02X:%02X:%02X:%02X:%02X:%02X",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return str;
}

int cgi_str2mac(char *str, unsigned char *mac)
{
	int i = 0, j = 0;
	unsigned char v = 0;

	for (i = 0; i < 17; i++) {
		if (str[i] >= '0' && str[i] <= '9') {
			v = str[i] - '0';
		} else if (str[i] >= 'a' && str[i] <= 'f') {
			v = str[i] - 'a' + 10;
		} else if (str[i] >= 'A' && str[i] <= 'F') {
			v = str[i] - 'A' + 10;
		} else if (str[i] == ':' || str[i] == '-' ||
					str[i] == ',' || str[i] == '\r' ||
					str[i] == '\n') {
			continue;
		} else if (str[i] == '\0') {
			return 0;
		} else {
			return -1;
		}
		if (j%2)
			mac[j/2] += v;
		else
			mac[j/2] = v*16;
		j++;
		if (j/2 > 5)
			break;
	}
	return 0;
}

int calc_uptime(struct uptime_record_info *t, int *ontime)
{
	time_t now_time;
	long sys_time = sys_uptime();
	long uptime;

	time(&now_time);
	if (!t->time) {
		*ontime = 0;
		CGI_PRINTF("uptime time(%d) err\n", t->time);
		return 0;
	} else if (t->flag == UPTIME_NET) {
		*ontime = now_time - t->time;
		return t->time;
	} else if (t->flag == UPTIME_SYS) {
		uptime = now_time - sys_time + t->time;
		*ontime = sys_time - t->time;
		return uptime >= 0 ? uptime : 0;
	} else {
		*ontime = 0;
		CGI_PRINTF("uptime flag(%d) err\n", t->flag);
		return 0;
	}
}

struct json_object *get_app_json(struct host_app_dump_info *app)
{
	int time = 0, i = 0, ontime = 0;
	struct json_object *obj, *papp;
	char app_flag[HARF_MAX + 1] = {0,};

	papp = json_object_new_object();
	
	obj = json_object_new_int(app->appid);
	json_object_object_add(papp, "id", obj);
	
	obj = json_object_new_int(app->down_speed/1024);
	json_object_object_add(papp, "speed", obj);

	obj = json_object_new_int(app->up_speed/1024);
	json_object_object_add(papp, "up_speed", obj);

	obj = json_object_new_uint64(app->up_bytes/1024);
	json_object_object_add(papp, "up_bytes", obj);

	obj = json_object_new_uint64(app->down_bytes/1024);
	json_object_object_add(papp, "down_bytes", obj);

	time = calc_uptime(&app->uptime, &ontime);
	obj = json_object_new_int(time);
	json_object_object_add(papp, "uptime", obj);

	if (!igd_test_bit(HARF_ONLINE, app->flag))
		ontime = 0;
	obj = json_object_new_int(ontime);
	json_object_object_add(papp, "ontime", obj);

	memset(app_flag, 0, sizeof(app_flag));
	for (i = 0; i < HARF_MAX; i++) {
		app_flag[i] = \
			igd_test_bit(i, app->flag) ? 'T' : 'F';
	}
	obj = json_object_new_string(app_flag);
	json_object_object_add(papp, "flag", obj);

	return papp;
}
	
struct json_object *get_host_json(struct host_dump_info *host, char *callip)
{
	unsigned long speed;
	int time = 0, i = 0, ontime = 0;
	struct json_object *obj, *phost;
	char host_flag[HIRF_MAX + 3] = {0,}, *ptr = NULL;

	phost = json_object_new_object();

	obj= json_object_new_string(cgi_mac2str(host->mac));
	json_object_object_add(phost, "mac", obj);

	obj= json_object_new_string(
		host->ip[0].s_addr ? inet_ntoa(host->ip[0]) : "");
	json_object_object_add(phost, "ip", obj);

	speed = (host->ls.down && (host->down_speed \
		> host->ls.down)) ? host->ls.down : host->down_speed;
	obj= json_object_new_int(speed/1024);
	json_object_object_add(phost, "speed", obj);

	speed = (host->ls.up && (host->up_speed \
		> host->ls.up)) ? host->ls.up : host->up_speed;
	obj= json_object_new_int(speed/1024);
	json_object_object_add(phost, "up_speed", obj);

	obj= json_object_new_uint64(host->up_bytes/1024);
	json_object_object_add(phost, "up_bytes", obj);

	obj= json_object_new_uint64(host->down_bytes/1024);
	json_object_object_add(phost, "down_bytes", obj);

	obj= json_object_new_string(host->name);
	json_object_object_add(phost, "name", obj);

	obj = json_object_new_int(host->vender);
	json_object_object_add(phost, "vendor", obj);

	obj = json_object_new_int(host->os_type);
	json_object_object_add(phost, "ostype", obj);

	time = (int)calc_uptime(&host->uptime, &ontime);
	obj = json_object_new_int(time);
	json_object_object_add(phost, "uptime", obj);

	if (!igd_test_bit(HIRF_ONLINE, host->flag))
		ontime = 0;
	obj = json_object_new_int(ontime);
	json_object_object_add(phost, "ontime", obj);

	obj = json_object_new_int(host->mode);
	json_object_object_add(phost, "mode", obj);

	obj = json_object_new_int(host->pic);
	json_object_object_add(phost, "pic", obj);

	memset(host_flag, 0, sizeof(host_flag));
	host_flag[0] = 'F'; 
	for (i = 0; i < 1 /*IGD_HOST_IP_MX*/; i++) {
		if (host->ip[i].s_addr == 0)
			continue;
		ptr = inet_ntoa(host->ip[i]);
		if (ptr != NULL && callip != NULL \
				&& strcmp(ptr, callip) == 0) {
			host_flag[0] = 'T';
		}
	}
	for (i = 0; i < HIRF_MAX; i++) {
		host_flag[i + 1] = \
			igd_test_bit(i, host->flag) ? 'T' : 'F';
	}
	obj = json_object_new_string(host_flag);
	json_object_object_add(phost, "flag", obj);

	return phost;
}

int cgi_sys_main_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	int status = 0;
	char cmd[128] = {0}, *login = NULL;
	int ret = 0, i = 0, j = 0;
	int host_nr = 0, app_nr = 0;
	unsigned long down_speed = 0, up_speed = 0;
	struct json_object *obj, *ts, *t, *app, *apps;
	struct host_dump_info host[IGD_HOST_MX];
	struct host_app_dump_info host_app[IGD_HOST_APP_MX];
	struct qos_conf qos_config;
	struct if_statistics statis;
	struct iface_info ifinfo;
	
	login = con_value_get(con, "login");
	if (login && atoi(login))
		CHECK_LOGIN;
	if (mu_msg(QOS_PARAM_SHOW, NULL, 0, &qos_config, sizeof(qos_config)))
		return CGI_ERR_FAIL;

	obj = json_object_new_int(qos_config.down/8);
	json_object_object_add(response, "total_speed", obj);

	obj = json_object_new_int(qos_config.up/8);
	json_object_object_add(response, "total_upspeed", obj);

	status = 1;
	if (mu_msg(IF_MOD_IFACE_INFO, &status, sizeof(int), &ifinfo, sizeof(ifinfo)))
		return CGI_ERR_FAIL;
	if (!mu_msg(SYSTEM_MOD_DEV_CHECK, NULL, 0, &status, sizeof(int))) {
		if (ifinfo.net && !status) {
			snprintf(cmd, 128, "qos_rule.status.status=%d", !status);
			uuci_set(cmd);
		}
	}
	obj = json_object_new_boolean(ifinfo.net);
	json_object_object_add(response, "connected", obj);
	
	obj = json_object_new_int(sys_uptime());
	json_object_object_add(response, "ontime", obj);

	host_nr = mu_msg(IGD_HOST_DUMP, NULL, 0, host, sizeof(host));
	if (host_nr < 0) {
		CGI_PRINTF("dump host err, ret:%d\n", host_nr);
		return CGI_ERR_FAIL;
	}

	ts = json_object_new_array();
	for (i = 0; i < host_nr; i++) {
		if (!qos_config.enable)
			igd_clear_bit(HIRF_ISKING, host[i].flag);
		t = get_host_json(&host[i], con->ip_from);

		apps = json_object_new_array();
		app_nr = mu_msg(IGD_HOST_APP_DUMP,
			host[i].mac, 6, host_app, sizeof(host_app));
		if (app_nr >= 0) {
			for (j = 0; j < app_nr; j++) {
				if (!igd_test_bit(HARF_ONLINE, host_app[j].flag))
					continue;
				app = get_app_json(&host_app[j]);
				json_object_array_add(apps, app);
			}
		} else {
			CGI_PRINTF("dump app err, ret:%d\n", app_nr);
		}
		json_object_object_add(t, "apps", apps);
		json_object_array_add(ts, t);
		up_speed += host[i].up_speed;
		down_speed += host[i].down_speed;
	}

	up_speed /= 1024;
	down_speed /= 1024;
	get_if_statistics(1, &statis);
	ret = (int)(statis.in.all.speed/1024);
	obj = json_object_new_int(ret > down_speed ? ret : down_speed);
	json_object_object_add(response, "cur_speed", obj);
	ret = (int)(statis.out.all.speed/1024);
	obj = json_object_new_int(ret > up_speed ? ret : up_speed);
	json_object_object_add(response, "up_speed", obj);

 	obj = json_object_new_uint64(statis.in.all.bytes/1024);
	json_object_object_add(response, "down_bytes", obj);

 	obj = json_object_new_uint64(statis.out.all.bytes/1024);
	json_object_object_add(response, "up_bytes", obj);

	json_object_object_add(response, "terminals", ts);
	return 0;
}

int cgi_net_host_app_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *pmac, *pact, *pappid, *pver;
	unsigned char mac[6] = {0,};
	struct json_object *t = NULL, *tmp = NULL, *apps = NULL, *app = NULL, *obj = NULL;
	struct host_dump_info host;
	struct host_app_dump_info host_app[IGD_HOST_APP_MX];
	int app_nr = 0, i = 0;
	unsigned int appid = 0;

	pmac = con_value_get(con, "mac");
	pact = con_value_get(con, "act");
	pver = con_value_get(con, "ver");
	if (!pmac || cgi_str2mac(pmac, mac)) {
		CGI_PRINTF("input err, %p\n", pmac);
		return CGI_ERR_INPUT;
	}

	if (!pact || strstr(pact, "host")) {
		if (mu_msg(IGD_HOST_DUMP, mac, 6, &host, sizeof(host)) < 0) {
			CGI_PRINTF("get host fail\n");
			return CGI_ERR_FAIL;
		}
		t = get_host_json(&host, con->ip_from);

		obj = json_object_new_int(host.ls.down);
		json_object_object_add(t, "ls", obj);
		obj = json_object_new_int(host.ls.up);
		json_object_object_add(t, "ls_up", obj);

		if (!pver) { // for 1.0 app
			tmp = json_object_new_array();
			json_object_object_add(t, "lm", tmp);

			tmp = json_object_new_object();
			obj = json_object_new_array();
			json_object_object_add(tmp, "week", obj);

			obj = json_object_new_int(0);
			json_object_object_add(tmp, "sh", obj);

			obj = json_object_new_int(0);
			json_object_object_add(tmp, "sm", obj);

			obj = json_object_new_int(0);
			json_object_object_add(tmp, "eh", obj);

			obj = json_object_new_int(0);
			json_object_object_add(tmp, "em", obj);

			obj = json_object_new_int(0);
			json_object_object_add(tmp, "st", obj);

			obj = json_object_new_int(0);
			json_object_object_add(tmp, "ft", obj);

			json_object_object_add(t, "lt", tmp);
		}

		json_object_object_add(response, "host", t);
	}

	if (!pact || strstr(pact, "app")) {
		pappid = con_value_get(con, "appid");
		if (pappid)
			appid = (unsigned int)atoll(pappid);

		apps = json_object_new_array();
		app_nr = mu_msg(IGD_HOST_APP_DUMP, mac, 6, host_app, sizeof(host_app));
		if (app_nr < 0) {
			CGI_PRINTF("dump app err, ret:%d\n", app_nr);
			return CGI_ERR_FAIL;
		}

		for (i = 0; i < app_nr; i++) {
			if (appid && (appid != host_app[i].appid))
				continue;

			app = get_app_json(&host_app[i]);
			if (appid) {
				tmp = json_object_new_object();

				obj = json_object_new_int(host_app[i].lt.start_hour);
				json_object_object_add(tmp, "sh", obj);

				obj = json_object_new_int(host_app[i].lt.start_min);
				json_object_object_add(tmp, "sm", obj);

				obj = json_object_new_int(host_app[i].lt.end_hour);
				json_object_object_add(tmp, "eh", obj);

				obj = json_object_new_int(host_app[i].lt.end_min);
				json_object_object_add(tmp, "em", obj);

				json_object_object_add(app, "lt", tmp);
			}

			json_object_array_add(apps, app);
		}

		if (t == NULL)
			json_object_object_add(response, "apps", apps);
		else
			json_object_object_add(t, "apps", apps);
	}
	if (!t && !apps)
		return CGI_ERR_INPUT;
	return 0;
}

enum HOST_IF_TYPE {
	HIT_IP = 0,
	HIT_NAME,
	HIT_VENDER,
	HIT_OSTYPE,
	HIT_SPEED,
	HIT_BYTES,
	HIT_TIME,
	HIT_LS,

	HIT_MAX, //must last
};

int cgi_net_host_if_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	unsigned long speed;
	int host_nr, i, j, time, ontime;
	char if_data[HIT_MAX], host_flag[HIRF_MAX + 3] = {0,}, *pif_data, *ptr;
	struct json_object *phost, *obj, *ts;
	struct host_dump_info *host, hdi[HOST_MX];

	pif_data = con_value_get(con, "if");
	if (!pif_data)
		return CGI_ERR_INPUT;

	memset(if_data, 0, sizeof(if_data));
	for (i = 0; i < HIT_MAX; i++) {
		if (pif_data[i] == '\0')
			break;
		else if (pif_data[i] == 'T')
			if_data[i] = 1;
	}

	host_nr = mu_msg(IGD_HOST_DUMP, NULL, 0, hdi, sizeof(hdi));
	if (host_nr < 0) {
		CGI_PRINTF("dump host err, ret:%d\n", host_nr);
		return CGI_ERR_FAIL;
	}

	ts = json_object_new_array();
	for (j = 0; j < host_nr; j++) {
		host = &hdi[j];
		phost = json_object_new_object();
		obj= json_object_new_string(cgi_mac2str(host->mac));
		json_object_object_add(phost, "mac", obj);

		memset(host_flag, 0, sizeof(host_flag));
		host_flag[0] = 'F';
		for (i = 0; i < 1 /*HOST_NUM_MAX*/; i++) {
			if (host->ip[i].s_addr == 0)
				continue;
			ptr = inet_ntoa(host->ip[i]);
			if (ptr != NULL && con->ip_from != NULL \
					&& strcmp(ptr, con->ip_from) == 0) {
				host_flag[0] = 'T';
			}
		}
		for (i = 0; i < HIRF_MAX ; i++) {
			if (igd_test_bit(i, host->flag))
				host_flag[i + 1] = 'T';
			else
				host_flag[i + 1] = 'F';
		}
		obj = json_object_new_string(host_flag);
		json_object_object_add(phost, "flag", obj);

		if (if_data[HIT_IP]) {
			if (host->ip[0].s_addr)
				obj= json_object_new_string(inet_ntoa(host->ip[0]));
			else
				obj= json_object_new_string("");
			json_object_object_add(phost, "ip", obj);
		}
		if (if_data[HIT_NAME]) {
			obj= json_object_new_string(host->name);
			json_object_object_add(phost, "name", obj);
		}
		if (if_data[HIT_VENDER]) {
			obj = json_object_new_int(host->vender);
			json_object_object_add(phost, "vendor", obj);
		}
		if (if_data[HIT_OSTYPE]) {
			obj = json_object_new_int(host->os_type);
			json_object_object_add(phost, "ostype", obj);
		}

		if (if_data[HIT_SPEED]) {
			speed = (host->ls.down && (host->down_speed \
				> host->ls.down)) ? host->ls.down : host->down_speed;
			obj= json_object_new_int(speed/1024);
			json_object_object_add(phost, "speed", obj);
			
			speed = (host->ls.up && (host->up_speed \
				> host->ls.up)) ? host->ls.up : host->up_speed;
			obj= json_object_new_int(speed/1024);
			json_object_object_add(phost, "up_speed", obj);
		}
		if (if_data[HIT_BYTES]) {
			obj= json_object_new_int(host->up_bytes);
			json_object_object_add(phost, "up_bytes", obj);
			obj= json_object_new_int(host->down_bytes);
			json_object_object_add(phost, "down_bytes", obj);
		}
		if (if_data[HIT_TIME]) {
			time = (int)calc_uptime(&host->uptime, &ontime);
			obj = json_object_new_int(time);
			json_object_object_add(phost, "uptime", obj);
			
			if (igd_test_bit(HIRF_ONLINE, host->flag)) {
				obj = json_object_new_int(ontime);
			} else {
				obj = json_object_new_int(0);
			}
			json_object_object_add(phost, "ontime", obj);
		}
		if (if_data[HIT_LS]) {
			obj = json_object_new_int(host->ls.down);
			json_object_object_add(phost, "ls", obj);
			obj = json_object_new_int(host->ls.up);
			json_object_object_add(phost, "ls_up", obj);
		}
		json_object_array_add(ts, phost);
	}
	json_object_object_add(response, "terminals", ts);
	return 0;
}

#define CON_GET_MAC(pmac, con, mac) do {\
	pmac = con_value_get(con, "mac");\
	if (!pmac || cgi_str2mac(pmac, (unsigned char *)mac)) {\
		CGI_PRINTF("mac is NULL\n");\
		return CGI_ERR_INPUT;\
	}\
} while(0)

#define CON_GET_ACT(pact, con, act) do {\
	pact = con_value_get(con, "act");\
	if (!pact) {\
		CGI_PRINTF("act is NULL\n");\
		return CGI_ERR_INPUT;\
	} else if (!strcmp(pact, "on") || !strcmp(pact, "add")) {\
		act = NLK_ACTION_ADD;\
	} else if (!strcmp(pact, "off") || !strcmp(pact, "del")) {\
		act = NLK_ACTION_DEL;\
	} else if (!strcmp(pact, "dump")) {\
		act = NLK_ACTION_DUMP;\
	} else if (!strcmp(pact, "mod")) {\
		act = NLK_ACTION_MOD;\
	} else {\
		CGI_PRINTF("act err, act:%s\n", pact);\
		return CGI_ERR_INPUT;\
	}\
} while(0)

#define CON_GET_INT(pdata, con, data, str) do {\
	pdata = con_value_get(con, str);\
	if (!pdata) {\
		CGI_PRINTF("%s is NULL\n", str);\
		return CGI_ERR_INPUT;\
	}\
	data = atoi(pdata);\
} while(0)

#define CON_GET_STR(pdata, con, data, str) do {\
	pdata = con_value_get(con, str);\
	if (!pdata || !strlen(pdata)) {\
		CGI_PRINTF("%s is NULL\n", str);\
		return CGI_ERR_INPUT;\
	}\
	arr_strcpy(data, pdata);\
} while(0)

#define CON_GET_CHECK_INT(pdata, con, data, str, max) do {\
	CON_GET_INT(pdata, con, data, str);\
	if (data > max){\
		CGI_PRINTF("%s > max(%d)\n", str, max);\
		return CGI_ERR_INPUT;\
	}\
}while(0)

int cgi_net_host_mode_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_INT(ptr, con, info.v.mode, "mode");
	if (mu_msg(IGD_HOST_SET_MODE, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;
	return 0;
}

static int cgi_get_time_comm(struct time_comm *t, connection_t *con)
{
	int i, st, et, nt;
	char *ptr;
	struct tm *ntime;

	CON_GET_CHECK_INT(ptr, con, t->start_hour, "sh", 23);
	CON_GET_CHECK_INT(ptr, con, t->start_min, "sm", 59);
	CON_GET_CHECK_INT(ptr, con, t->end_hour, "eh", 23);
	CON_GET_CHECK_INT(ptr, con, t->end_min, "em", 59);

	st = t->start_hour*60 + t->start_min;
	et = t->end_hour*60 + t->end_min;

	if (st == et)
		return CGI_ERR_INPUT;

	ptr = con_value_get(con, "week");
	if (!ptr) {
		CGI_PRINTF("input err, %p\n", ptr);
		return CGI_ERR_INPUT;
	}
	for (i = 0; i < 7; i++) {
		if (*(ptr + i) == '\0')
			break;
		if (*(ptr + i) == '1')
			CGI_BIT_SET(t->day_flags, i);
	}

	if (t->day_flags) {
		t->loop = 1;
		return 0;
	}
	t->loop = 0;

	ntime = get_tm();
	if (!ntime) {
		CGI_PRINTF("get time fail\n");
		return CGI_ERR_FAIL;
	}
	CGI_BIT_SET(t->day_flags, ntime->tm_wday);
	if (st > et)
		return 0;

	nt = ntime->tm_hour*60 + ntime->tm_min;
	if (et < nt) {
		CGI_PRINTF("time out\n");
		return CGI_ERR_TIMEOUT;
	}
	return 0;
}

int cgi_net_host_lmt_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr;
	struct host_set_info info;
	struct app_mod_dump *dump;
	int nr, i, j;
	struct json_object *obj, *lmt, *mt, *tmp;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);
	if (info.act == NLK_ACTION_DUMP) {
		dump = malloc(sizeof(*dump)*IGD_APP_MOD_TIME_MX);
		if (!dump)
			return CGI_ERR_MALLOC;
		nr = mu_msg(IGD_HOST_APP_MOD_ACTION, &info, sizeof(info),
			dump, IGD_APP_MOD_TIME_MX*sizeof(*dump));
		if (nr < 0) {
			free(dump);
			CGI_PRINTF("mu msg err, ret:%d\n", nr);
			return CGI_ERR_FAIL;
		}
		lmt = json_object_new_array();
		for (i = 0; i < nr; i++) {
			mt = json_object_new_object();
			
			obj = json_object_new_int(dump[i].time.start_hour);
			json_object_object_add(mt, "sh", obj);

			obj = json_object_new_int(dump[i].time.start_min);
			json_object_object_add(mt, "sm", obj);

			obj = json_object_new_int(dump[i].time.end_hour);
			json_object_object_add(mt, "eh", obj);

			obj = json_object_new_int(dump[i].time.end_min);
			json_object_object_add(mt, "em", obj);

			tmp = json_object_new_array();
			for (j = 0; j < 7; j++) {
				if (!dump[i].time.loop || 
					!CGI_BIT_TEST(dump[i].time.day_flags, j))
					continue;
				obj = json_object_new_int(j);
				json_object_array_add(tmp, obj);
			}
			json_object_object_add(mt, "week", tmp);

			tmp = json_object_new_array();
			for (j = 0; j < L7_MID_MX; j++) {
				if (!igd_test_bit(j, dump[i].mid_flag))
					continue;
				obj = json_object_new_int(j);
				json_object_array_add(tmp, obj);
			}
			json_object_object_add(mt, "mid", tmp);

			obj = json_object_new_int(dump[i].enable);
			json_object_object_add(mt, "enable", obj);

			json_object_array_add(lmt, mt);
		}
		json_object_object_add(response, "lmt", lmt);
		free(dump);
		return 0;
	}

	CON_GET_INT(ptr, con, info.v.app_mod.enable, "enable");
	nr = cgi_get_time_comm(&info.v.app_mod.time, con);
	if (nr) {
		CGI_PRINTF("get time com fail\n");
		return nr;
	}

	ptr = con_value_get(con, "mid");
	if (!ptr) {
		CGI_PRINTF("input err, %p\n", ptr);
		return CGI_ERR_INPUT;
	}
	for (i = 0; i < L7_MID_MX; i++) {
		if (*(ptr + i) == '\0')
			break;
		if (*(ptr + i) == '1')
			igd_set_bit(i, info.v.app_mod.mid_flag);
	}

	nr = mu_msg(IGD_HOST_APP_MOD_ACTION, &info, sizeof(info), NULL, 0);
	if (nr == -2)
		return CGI_ERR_FULL;
	else if (nr < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_host_ls_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_INT(ptr, con, info.v.ls.down, "speed");
	ptr = con_value_get(con, "up_speed");
	if (ptr)
		info.v.ls.up = atoi(ptr);

	if (mu_msg(IGD_HOST_SET_LIMIT_SPEED, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_host_study_time_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr;
	struct host_set_info info;
	struct study_time_dump *dump;
	int nr = 0, i, j;
	struct json_object *t_arr, *t_obj, *t_day, *obj;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);

	if (info.act == NLK_ACTION_DUMP) {
		dump = malloc(sizeof(*dump)*IGD_STUDY_TIME_NUM);
		if (!dump)
			return CGI_ERR_MALLOC;
		nr = mu_msg(IGD_HOST_STUDY_TIME, &info, sizeof(info),
			dump, sizeof(*dump)*IGD_STUDY_TIME_NUM);
		if (nr < 0) {
			free(dump);
			CGI_PRINTF("mu msg err, ret:%d\n", nr);
			return CGI_ERR_FAIL;
		}
		t_arr = json_object_new_array();
		for (i = 0; i < nr; i ++) {
			t_obj = json_object_new_object();
			
			obj = json_object_new_int(dump[i].enable);
			json_object_object_add(t_obj, "enable", obj);

			obj = json_object_new_int(dump[i].time.start_hour);
			json_object_object_add(t_obj, "sh", obj);

			obj = json_object_new_int(dump[i].time.start_min);
			json_object_object_add(t_obj, "sm", obj);

			obj = json_object_new_int(dump[i].time.end_hour);
			json_object_object_add(t_obj, "eh", obj);

			obj = json_object_new_int(dump[i].time.end_min);
			json_object_object_add(t_obj, "em", obj);

			t_day = json_object_new_array();
			for (j = 0; j < 7; j++) {
				if (!dump[i].time.loop || 
					!CGI_BIT_TEST(dump[i].time.day_flags, j))
					continue;
				obj = json_object_new_int(j);
				json_object_array_add(t_day, obj);
			}
			json_object_object_add(t_obj, "week", t_day);
			json_object_array_add(t_arr, t_obj);
		}
		json_object_object_add(response, "study_time", t_arr);
		free(dump);
		return 0;
	}

	CON_GET_INT(ptr, con, info.v.study_time.enable, "enable");
	nr = cgi_get_time_comm(&info.v.study_time.time, con);
	if (nr) {
		CGI_PRINTF("get time com fail\n");
		return nr;
	}

	nr = mu_msg(IGD_HOST_STUDY_TIME, &info, sizeof(info), NULL, 0);
	if (nr == -2)
		return CGI_ERR_FULL;
	else if (nr < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_host_king_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);

	if (mu_msg(IGD_HOST_SET_KING, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;

	return 0;
}

int cgi_net_host_black_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);

	if (mu_msg(IGD_HOST_SET_BLACK, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;

	return 0;
}

int cgi_net_app_black_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);
	CON_GET_INT(ptr, con, info.appid, "appid");

	if (IGD_TEST_STUDY_URL(info.appid))
		return CGI_ERR_NOSUPPORT;
	if (info.appid == IGD_CHUYUN_APPID)
		return CGI_ERR_NOSUPPORT;

	if (mu_msg(IGD_HOST_SET_APP_BLACK, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_host_nick_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	int len;
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);

	ptr = con_value_get(con, "nick");
	if (ptr) {
		len = strlen(ptr);
		if (len >= sizeof(info.v.name)) {
			return CGI_ERR_INPUT;
		} else if (len) {
			arr_strcpy(info.v.name, ptr);
		}
	}
	if (mu_msg(IGD_HOST_SET_NICK, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_app_lt_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;
	struct time_comm *t = &info.v.time;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);
	CON_GET_INT(ptr, con, info.appid, "appid");
	if (IGD_TEST_STUDY_URL(info.appid))
		return CGI_ERR_NOSUPPORT;
	if (info.appid == IGD_CHUYUN_APPID)
		return CGI_ERR_NOSUPPORT;

	if (info.act == NLK_ACTION_ADD) {
		CON_GET_CHECK_INT(ptr, con, t->start_hour, "sh", 23);
		CON_GET_CHECK_INT(ptr, con, t->start_min, "sm", 59);
		CON_GET_CHECK_INT(ptr, con, t->end_hour, "eh", 23);
		CON_GET_CHECK_INT(ptr, con, t->end_min, "em", 59);
		t->day_flags = 127;
		t->loop = 0;
	}

	if (mu_msg(IGD_HOST_SET_APP_LIMIT_TIME, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_study_url_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	struct host_set_info info;
	struct study_url_dump *dump;
	char *ptr;
	int start, num, nr, i;
	struct json_object *obj, *study_url, *study_urls;

	CHECK_LOGIN;
	memset(&info, 0, sizeof(info));

	CON_GET_ACT(ptr, con, info.act);
	if (info.act == NLK_ACTION_DUMP) {
		CON_GET_INT(ptr, con, start, "start");
		CON_GET_INT(ptr, con, num, "num");

		dump = malloc(sizeof(*dump)*IGD_STUDY_URL_SELF_NUM);
		if (!dump)
			return CGI_ERR_MALLOC;
		nr = mu_msg(IGD_STUDY_URL_ACTION, &info, sizeof(info),
			dump, sizeof(*dump)*IGD_STUDY_URL_SELF_NUM);
		if (nr < 0) {
			free(dump);
			CGI_PRINTF("mu msg err, ret:%d\n", nr);
			return CGI_ERR_FAIL;
		}

		study_urls = json_object_new_array();
		for (i = 0; i < nr; i++) {
			if ((i + 1) < start)
				continue;
			study_url = json_object_new_object();

			obj = json_object_new_int(dump[i].id);
			json_object_object_add(study_url, "id", obj);

			obj = json_object_new_string(dump[i].name);
			json_object_object_add(study_url, "name", obj);

			obj = json_object_new_string(dump[i].url);
			json_object_object_add(study_url, "url", obj);

			json_object_array_add(study_urls, study_url);
			num--;
			if (num <= 0)
				break;
		}
		json_object_object_add(response, "study_url", study_urls);
		free(dump);
		return 0;
	}
	return CGI_ERR_INPUT;
}

int cgi_net_host_study_url_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr;
	int nr, i, self = 0, len;
	struct study_url_dump *dump;
	struct host_set_info info;
	struct json_object *obj, *urls;
	unsigned char dataup[1024] = {0,};

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_ACT(ptr, con, info.act);
	CON_GET_MAC(ptr, con, info.mac);

	if (info.act == NLK_ACTION_DUMP) {
		dump = malloc(sizeof(*dump)*IGD_STUDY_URL_MX);
		if (!dump)
			return CGI_ERR_MALLOC;
		nr = mu_msg(IGD_HOST_STUDY_URL_ACTION, &info, sizeof(info),
			dump, sizeof(*dump)*IGD_STUDY_URL_MX);
		if (nr < 0) {
			free(dump);
			CGI_PRINTF("mu msg err, ret:%d\n", nr);
			return CGI_ERR_FAIL;
		}
		urls = json_object_new_array();
		for (i = 0; i < nr; i ++) {
			obj = json_object_new_int(dump[i].id);
			json_object_array_add(urls, obj);
		}
		json_object_object_add(response, "study_url", urls);
		free(dump);
		return 0;
	}

	if (info.act == NLK_ACTION_DEL) {
		CON_GET_INT(ptr, con, info.v.surl.id, "id");
		if (!IGD_TEST_STUDY_URL(info.v.surl.id)) {
			CGI_PRINTF("url id err, %d\n", info.v.surl.id);
			return CGI_ERR_INPUT;
		}
		if (mu_msg(IGD_HOST_STUDY_URL_ACTION, &info, sizeof(info), NULL, 0))
			return CGI_ERR_FAIL;
		if (mu_msg(IGD_STUDY_URL_ACTION, &info, sizeof(info), NULL, 0))
			return CGI_ERR_FAIL;
	} else if (info.act == NLK_ACTION_ADD) {
		CON_GET_STR(ptr, con, info.v.surl.name, "name");
		CON_GET_STR(ptr, con, info.v.surl.url, "url");
		ptr = con_value_get(con, "id");
		if (ptr) {
			info.v.surl.id = atoi(ptr);
		} else {
			self = 1;
			nr = mu_msg(IGD_STUDY_URL_ACTION, &info, sizeof(info), NULL, 0);
			if (nr == -2)
				return CGI_ERR_FULL;
			else if (nr <= 0) {
				CGI_PRINTF("add fail %d\n", nr);
				return CGI_ERR_FAIL;
			}
			info.v.surl.id = nr;
		}
		nr = mu_msg(IGD_HOST_STUDY_URL_ACTION, &info, sizeof(info), NULL, 0);
		if (nr < 0) {
			CGI_PRINTF("add fail %d\n", nr);
			return CGI_ERR_FAIL;
		}
		if (self) {
			obj = json_object_new_int(info.v.surl.id);
			json_object_object_add(response, "id", obj);
		}
	}

	if (IGD_SELF_STUDY_URL(info.v.surl.id)) {
		if (info.act == NLK_ACTION_ADD) {
			CC_PUSH2(dataup, 2, CSO_REQ_SITE_CUSTOM);
			i = 8;
			CC_PUSH4(dataup, i, info.v.surl.id);
			i += 4;;
			len = strlen(info.v.surl.url);
			CC_PUSH1(dataup, i, len);
			i += 1;
			CC_PUSH_LEN(dataup, i, info.v.surl.url, len);
			i += len;
			len = strlen(info.v.surl.name);
			CC_PUSH1(dataup, i, len);
			i += 1;
			CC_PUSH_LEN(dataup, i, info.v.surl.name, len);
			i += len;
			CC_PUSH2(dataup, 0, i);
			CC_MSG_ADD(dataup, i);
		}
	} else {
		CC_PUSH2(dataup, 2, CSO_REQ_SITE_RECOMMAND);
		i = 8;
		CC_PUSH4(dataup, i, info.v.surl.id);
		i += 4;
		CC_PUSH1(dataup, i, (info.act == NLK_ACTION_ADD) ? 1 : 0);
		i += 1;
		CC_PUSH2(dataup, 0, i);
		CC_MSG_ADD(dataup, i);
	}
	return 0;
}

int cgi_net_host_del_history_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);

	if (mu_msg(IGD_HOST_DEL_HISTORY, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;

	if (mu_msg(NAT_MOD_MAC_DEL, info.mac, 6, NULL, 0) < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_host_dbg_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	if (mu_msg(IGD_HOST_DBG_FILE, NULL, 0, NULL, 0) < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_sys_rconf_ver_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char buf[512] = {0,}, *ptr = NULL;
	FILE *fp = NULL;
	struct json_object *rconf, *obj;

	fp = fopen(RCONF_CHECK, "r");
	if (!fp)
		return CGI_ERR_FAIL;

	rconf = json_object_new_object();
	while(1) {
		memset(buf, 0, sizeof(buf));
		if (!fgets(buf, sizeof(buf) - 1, fp))
			break;
		ptr = strrchr(buf, '\n');
		if (ptr)
			*ptr = '\0';
		ptr = strrchr(buf, '\r');
		if (ptr)
			*ptr = '\0';
		ptr = strchr(buf, ':');
		if (!ptr)
			continue;
		*ptr = '\0';
		obj = json_object_new_string(ptr + 1);
		json_object_object_add(rconf, buf, obj);
	}
	json_object_object_add(response, "rconf", rconf);
	return 0;
}

int cgi_net_host_url_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr, *ptype;
	struct host_set_info info;
	struct host_url_dump *dump;
	int nr, i;
	struct json_object *url_type, *obj;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);
	ptype = con_value_get(con, "type");
	if (!ptype) {
		CGI_PRINTF("type is null\n");
		return CGI_ERR_INPUT;
	} else if (!strcmp(ptype, "black")) {
		info.v.bw_url.type = RLT_URL_BLACK;
	} else if (!strcmp(ptype, "white")) {
		info.v.bw_url.type = RLT_URL_WHITE;
	} else {
		CGI_PRINTF("type is err, %s\n", ptype);
		return CGI_ERR_INPUT;
	}
	if (info.act == NLK_ACTION_DUMP) {
		dump = malloc(sizeof(*dump)*IGD_HOST_URL_MX);
		if (!dump)
			return CGI_ERR_MALLOC;
		nr = mu_msg(IGD_HOST_URL_ACTION, &info, sizeof(info),
			dump, IGD_HOST_URL_MX*sizeof(*dump));
		if (nr < 0) {
			free(dump);
			CGI_PRINTF("mu msg err, ret:%d\n", nr);
			return CGI_ERR_FAIL;
		}
		url_type = json_object_new_array();
		for (i = 0; i < nr; i++) {
			obj = json_object_new_string(dump[i].url);
			json_object_array_add(url_type, obj);
		}
		json_object_object_add(response, ptype, url_type);
		free(dump);
		return 0;
	}
	CON_GET_STR(ptr, con, info.v.bw_url.url, "url");
 	nr = mu_msg(IGD_HOST_URL_ACTION, &info, sizeof(info), NULL, 0);
	if ((nr == -2) && (info.act == NLK_ACTION_ADD))
		return CGI_ERR_FULL;
	if ((nr == -2) && (info.act == NLK_ACTION_DEL))
		return CGI_ERR_NONEXIST;
	else if (nr < 0)
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_study_url_var_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *opt, grp_name[32];
	struct in_addr ip;
	struct host_set_info info;
	struct study_url_dump *dump;
	struct json_object *url_group, *url_obj[10], *obj_one, *obj;
	int nr, i, cid;

	opt = con_value_get(con, "opt");
	if (!opt)
		return CGI_ERR_INPUT;

	if (con && con->ip_from) {
		CGI_PRINTF("host ip:%s\n", con->ip_from);
		inet_aton(con->ip_from, &ip);
	} else {
		return CGI_ERR_FAIL;
	}

	nr = mu_msg(IGD_HOST_IP2MAC, &ip, sizeof(ip), info.mac, sizeof(info.mac));
	if (nr < 0)
		return CGI_ERR_FAIL;
	CGI_PRINTF("host mac:%s\n", cgi_mac2str(info.mac));

	info.act = NLK_ACTION_DUMP;
	dump = malloc(sizeof(*dump)*IGD_STUDY_URL_MX);
	if (!dump)
		return CGI_ERR_MALLOC;
	nr = mu_msg(IGD_HOST_STUDY_URL_ACTION, &info, sizeof(info),
		dump, sizeof(*dump)*IGD_STUDY_URL_MX);
	if (nr < 0) {
		free(dump);
		CGI_PRINTF("mu msg err, ret:%d\n", nr);
		return CGI_ERR_FAIL;
	}

	memset(url_obj, 0, sizeof(url_obj));
	url_group = json_object_new_object();
	for (i = 0; i < nr; i++) {
		cid = (dump[i].id/(L7_APP_MX/10) - L7_MID_HTTP*10);
		if (cid < 0 || cid > 10)
			continue;
		if (!url_obj[cid])
			url_obj[cid] = json_object_new_array();
		if (cid == 0) {
			obj_one = json_object_new_object();
			obj = json_object_new_int(dump[i].id);
			json_object_object_add(obj_one, "id", obj);
			obj = json_object_new_string(dump[i].name);
			json_object_object_add(obj_one, "name", obj);
			obj = json_object_new_string(dump[i].url);
			json_object_object_add(obj_one, "url", obj);
			json_object_array_add(url_obj[cid], obj_one);
		} else {
			obj_one = json_object_new_int(dump[i].id);
			json_object_array_add(url_obj[cid], obj_one);
		}
	}

	for (i = 0; i < 10; i++) {
		if (!url_obj[i])
			continue;
		snprintf(grp_name, sizeof(grp_name) - 1, "T_%d", i);
		json_object_object_add(url_group, grp_name, url_obj[i]);
	}
	json_object_object_add(response, opt, url_group);
	free(dump);
	return 0;
}

int cgi_net_host_push_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_ACT(ptr, con, info.act);

	if (mu_msg(IGD_HOST_SET_ONLINE_PUSH, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;

	return 0;
}

int cgi_net_new_push_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info, back;
	struct json_object *obj;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_ACT(ptr, con, info.act);

	if (mu_msg(IGD_HOST_SET_NEW_PUSH, &info, sizeof(info), &back, sizeof(back)) < 0)
		return CGI_ERR_FAIL;

	if (info.act == NLK_ACTION_DUMP) {
		obj = json_object_new_int(back.v.new_push);
		json_object_object_add(response, "new_push", obj);
	}
	return 0;
}

int cgi_net_host_pic_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct host_set_info info;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_MAC(ptr, con, info.mac);
	CON_GET_INT(ptr, con, info.v.pic, "pic");

	if (mu_msg(IGD_HOST_SET_PIC, &info, sizeof(info), NULL, 0) < 0)
		return CGI_ERR_FAIL;

	return 0;
}

int cgi_sys_led_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	int act = 0, led = 0;
	char *ptr = NULL;
	struct json_object *obj = NULL;

	CHECK_LOGIN;

	CON_GET_ACT(ptr, con, act);
	if (mu_msg(SYSTME_MOD_SET_LED, &act, sizeof(act), &led, sizeof(led)) < 0)
		return CGI_ERR_FAIL;
	if (act == NLK_ACTION_DUMP) {
		obj = json_object_new_int(led);
		json_object_object_add(response, "led", obj);
	}
	return 0;
}

int cgi_host_nat_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	int act = 0, i = 0, num = 0;
	struct igd_nat_param inp[IGD_NAT_MAX];
	struct json_object *object, *array, *t;

	CHECK_LOGIN;

	CON_GET_ACT(ptr, con, act);
	CON_GET_MAC(ptr, con, inp[0].mac);
	if (act == NLK_ACTION_DUMP) {
		num = mu_msg(NAT_MOD_DUMP, inp[0].mac, sizeof(inp[0].mac), inp, sizeof(inp));
		if (num < 0)
			return CGI_ERR_FAIL;
		array = json_object_new_array();
		for (i = 0; i < num; i++) {
 			object = json_object_new_object();

			t = json_object_new_int(inp[i].out_port);
			json_object_object_add(object, "out_port", t);

			t = json_object_new_int(inp[i].in_port);
			json_object_object_add(object, "in_port", t);

			t = json_object_new_int(inp[i].proto);
			json_object_object_add(object, "proto", t);

			t = json_object_new_int(inp[i].enable);
			json_object_object_add(object, "enable", t);

			json_object_array_add(array, object);
		}
		json_object_object_add(response, "nat", array);
	} else {
		CON_GET_INT(ptr, con, inp[0].out_port, "out_port");
		CON_GET_INT(ptr, con, inp[0].in_port, "in_port");
		CON_GET_INT(ptr, con, inp[0].proto, "proto");
		CON_GET_INT(ptr, con, inp[0].enable, "enable");

		if (act == NLK_ACTION_ADD) {
			num = mu_msg(NAT_MOD_ADD, &inp[0], sizeof(inp[0]), NULL, 0);
			if (num == -2) {
				return (act == NLK_ACTION_ADD) ? CGI_ERR_EXIST : CGI_ERR_NONEXIST;
			} else if (num == -3) {
				return CGI_ERR_FULL;
			} else if (num < 0) {
				return CGI_ERR_FAIL;
			}
		} else if (act == NLK_ACTION_DEL) {
			num = mu_msg(NAT_MOD_DEL, &inp[0], sizeof(inp[0]), NULL, 0);
			if (num == -2) {
				return (act == NLK_ACTION_ADD) ? CGI_ERR_EXIST : CGI_ERR_NONEXIST;
			} else if (num == -3) {
				return CGI_ERR_FULL;
			} else if (num < 0) {
				return CGI_ERR_FAIL;
			}
		} else if (act == NLK_ACTION_MOD) {
			if (mu_msg(NAT_MOD_DEL, &inp[0], sizeof(inp[0]), NULL, 0) < 0) {
				CGI_PRINTF("mod del fail\n");
				return CGI_ERR_FAIL;
			}
			if (mu_msg(NAT_MOD_ADD, &inp[0], sizeof(inp[0]), NULL, 0) < 0) {
				CGI_PRINTF("mod add fail\n");
				return CGI_ERR_FAIL;
			}
		} else {
			return CGI_ERR_INPUT;
		}

	}
	return 0;
}

int cgi_sys_account_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	struct sys_account info, pre;

	CHECK_LOGIN;

	if (connection_is_set(con)) {
		CON_GET_STR(ptr, con, info.user, "user");
		CON_GET_STR(ptr, con, info.password, "password");
		if (strcmp(info.user, "admin"))
			return CGI_ERR_INPUT;
		if (mu_msg(SYSTME_MOD_GET_ACCOUNT, NULL, 0, &pre, sizeof(pre)))
			return CGI_ERR_FAIL;
		if (!strcmp(pre.password, info.password))
			return 0;
		if (mu_msg(SYSTME_MOD_SET_ACCOUNT, &info, sizeof(info), NULL, 0))
			return CGI_ERR_FAIL;
		server_clean(srv);
	}
	return 0;
}

int cgi_system_cmdline_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *cmd;

	CHECK_LOGIN;

	cmd = con_value_get(con, "cmd");
	if (!cmd || !strlen(cmd)) {
		CGI_PRINTF("%s is NULL\n", "cmd");
		return CGI_ERR_INPUT;
	}

	return mu_msg(SYSTME_MOD_SYS_CMD, cmd, strlen(cmd) + 1, NULL, 0);
}

#define SYSTEM_APP_NUM  500
struct sys_app_info {
	uint32_t appid;
	unsigned long down_speed;
	unsigned long up_speed;
	uint64_t up_bytes;
	uint64_t down_bytes;
	unsigned long flag[BITS_TO_LONGS(HARF_MAX)];
};

int cgi_sys_app_info_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	int host_nr, app_nr, sys_app_nr, i, j, k;
	struct host_dump_info host[IGD_HOST_MX];
	struct host_app_dump_info host_app[IGD_HOST_APP_MX];
	struct sys_app_info *sys_app, *sa = NULL;
	struct json_object *t_arr, *t_obj, *obj;
	char flag[HARF_MAX + 1] = {0,};

	CHECK_LOGIN;

	host_nr = mu_msg(IGD_HOST_DUMP, NULL, 0, host, sizeof(host));
	if (host_nr < 0) {
		CGI_PRINTF("dump host err, ret:%d\n", host_nr);
		return CGI_ERR_FAIL;
	}

	sys_app = malloc(sizeof(*sys_app) * SYSTEM_APP_NUM);
	if (!sys_app) {
		CGI_PRINTF("malloc fail, %d\n", sizeof(*sys_app) * SYSTEM_APP_NUM);
		return CGI_ERR_MALLOC;
	}
	sys_app_nr = 0;
	memset(sys_app, 0, sizeof(*sys_app) * SYSTEM_APP_NUM);

	for (i = 0; i < host_nr; i++) {
		app_nr = mu_msg(IGD_HOST_APP_DUMP,
			host[i].mac, sizeof(host[i].mac), host_app, sizeof(host_app));
		if (app_nr < 0) {
			CGI_PRINTF("dump app err, ret:%d, %s\n", 
				app_nr, cgi_mac2str(host[i].mac));
			continue;
		}
		for (j = 0; j < app_nr; j++) {
			if (sys_app_nr >= SYSTEM_APP_NUM)
				break;
			sa = NULL;
			for (k = 0; k < sys_app_nr; k++) {
				if (sys_app[k].appid == host_app[j].appid) {
					sa = &sys_app[k];
					break;
				}
			}
			if (!sa) {
				sa = &sys_app[sys_app_nr];
				sys_app_nr++;
			}
			sa->appid = host_app[j].appid;
			sa->up_speed += host_app[j].up_speed;
			sa->down_speed += host_app[j].down_speed;
			sa->up_bytes += host_app[j].up_bytes;
			sa->down_bytes += host_app[j].down_bytes;
			for (k = 0; k < BITS_TO_LONGS(HARF_MAX); k++)
				sa->flag[k] |= host_app[j].flag[k];
		}
	}

	t_arr = json_object_new_array();
	for (i = 0; i < sys_app_nr; i++) {
		t_obj = json_object_new_object();

		obj = json_object_new_int(sys_app[i].appid);
		json_object_object_add(t_obj, "appid", obj);

		obj = json_object_new_int(sys_app[i].up_speed/1024);
		json_object_object_add(t_obj, "up_speed", obj);
		obj = json_object_new_int(sys_app[i].down_speed/1024);
		json_object_object_add(t_obj, "down_speed", obj);

		obj = json_object_new_uint64(sys_app[i].up_bytes/1024);
		json_object_object_add(t_obj, "up_bytes", obj);
		obj = json_object_new_uint64(sys_app[i].down_bytes/1024);
		json_object_object_add(t_obj, "down_bytes", obj);

		for (k = 0; k < HARF_MAX; k++)
			flag[k] = igd_test_bit(k, sys_app[i].flag) ? 'T' : 'F';
		flag[k] = '\0';

		obj = json_object_new_string(flag);
		json_object_object_add(t_obj, "flag", obj);

		json_object_array_add(t_arr, t_obj);
	}
	json_object_object_add(response, "sys_app", t_arr);
	free(sys_app);
	return 0;
}

int cgi_net_intercept_url_black_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	struct host_set_info info, url;
	struct host_dump_info host[IGD_HOST_MX];
	struct host_intercept_url_dump url_dump[IGD_HOST_INTERCEPT_URL_MX];
	int host_nr, i, url_nr, j;
	char *ptr, *pmac, paddr[64], key[64];
	struct json_object *t_arr, *t_obj, *obj;

	CHECK_LOGIN;

	memset(&info, 0, sizeof(info));
	CON_GET_ACT(ptr, con, info.act);
	if (info.act == NLK_ACTION_DUMP) {
		pmac = con_value_get(con, "mac");
		if (pmac)
			pmac = cgi_str2mac(pmac, info.mac) ? NULL : (char *)info.mac;

		host_nr = mu_msg(IGD_HOST_DUMP, pmac, pmac ? 6 : 0, host, sizeof(host));
		if (host_nr < 0) {
			CGI_PRINTF("dump host err, ret:%d\n", host_nr);
			return CGI_ERR_FAIL;
		} else if (!host_nr && pmac) {
			host_nr = 1;
		}

		t_arr = json_object_new_array();
		for (i = 0; i < host_nr; i++) {
			memcpy(url.mac, host[i].mac, sizeof(url.mac));
			url.act = NLK_ACTION_DUMP;
			url_nr = mu_msg(IGD_HOST_INTERCEPT_URL_BLACK_ACTION,
					&url, sizeof(url), url_dump, sizeof(url_dump));
			if (url_nr < 0)
				continue;
			for (j = 0; j < url_nr; j++) {
				t_obj = json_object_new_object();

				obj = json_object_new_string(cgi_mac2str(host[i].mac));
				json_object_object_add(t_obj, "mac", obj);

				obj = json_object_new_string(host[i].name);
				json_object_object_add(t_obj, "name", obj);

				obj = json_object_new_string(url_dump[j].url);
				json_object_object_add(t_obj, "url", obj);

				obj = json_object_new_int(url_dump[j].time);
				json_object_object_add(t_obj, "time", obj);

				obj = json_object_new_string(url_dump[j].type);
				json_object_object_add(t_obj, "type", obj);

				obj = json_object_new_int(url_dump[j].sport);
				json_object_object_add(t_obj, "sport", obj);

				obj = json_object_new_int(url_dump[j].dport);
				json_object_object_add(t_obj, "dport", obj);

				obj = json_object_new_string(inet_ntoa(url_dump[j].saddr));
				json_object_object_add(t_obj, "saddr", obj);

				obj = json_object_new_string(inet_ntoa(url_dump[j].daddr));
				json_object_object_add(t_obj, "daddr", obj);

				json_object_array_add(t_arr, t_obj);
			}
		}
		json_object_object_add(response, "intercept_url_black", t_arr);
		return 0;
	}

	for (i = 0; i < 16; i++) {
		snprintf(key, sizeof(key), "mac_%d", i + 1);
		pmac = con_value_get(con, key);
		if (!pmac || cgi_str2mac(pmac, info.mac)) {
			CGI_PRINTF("mac is %p, i:%d\n", pmac, i);
			break;
		}
		snprintf(key, sizeof(key), "url_%d", i + 1);
		CON_GET_STR(ptr, con, info.v.intercept_url.url, key);

		snprintf(key, sizeof(key), "type_%d", i + 1);
		CON_GET_STR(ptr, con, info.v.intercept_url.type, key);

		snprintf(key, sizeof(key), "time_%d", i + 1);
		CON_GET_INT(ptr, con, info.v.intercept_url.time, key);

		snprintf(key, sizeof(key), "dport_%d", i + 1);
		CON_GET_INT(ptr, con, info.v.intercept_url.dport, key);

		snprintf(key, sizeof(key), "sport_%d", i + 1);
		CON_GET_INT(ptr, con, info.v.intercept_url.sport, key);

		snprintf(key, sizeof(key), "saddr_%d", i + 1);
		CON_GET_STR(ptr, con, paddr, key);
		inet_aton(paddr, &info.v.intercept_url.saddr);

		snprintf(key, sizeof(key), "daddr_%d", i + 1);
		CON_GET_STR(ptr, con, paddr, key);
		inet_aton(paddr, &info.v.intercept_url.daddr);

		url_nr = mu_msg(IGD_HOST_INTERCEPT_URL_BLACK_ACTION, &info, sizeof(info), NULL, 0);
		if (url_nr == -2)
			return CGI_ERR_FULL;
		else if (url_nr < 0)
			return CGI_ERR_FAIL;
	}
	return 0;
}

int cgi_net_abandon_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	return 0;
}

int cgi_net_app_queue_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	int i = 0;
	char *ptr, *pmac, *pqueue;
	unsigned char mid[L7_MID_MX];
	struct host_set_info info;
	struct json_object *t_arr, *t_obj;

	CHECK_LOGIN;

	memset(&mid, 0, sizeof(mid));
	memset(&info, 0, sizeof(info));

	CON_GET_ACT(ptr, con, info.act);
	pqueue = con_value_get(con, "queue");
	if (pqueue) {
		ptr = pqueue;
		while (ptr) {
			info.v.mid[i] = atoi(ptr);
			if (info.v.mid[i] < L7_MID_MX) {
				i++;
			} else {
				CGI_PRINTF("mid err, %s\n", pqueue);
				return CGI_ERR_INPUT;
			}
			ptr = strchr(ptr, ',');
			if (ptr)
				ptr++;
		}
	}
	pmac = con_value_get(con, "mac");
	if (pmac) {
		cgi_str2mac(pmac, info.mac);
		i = mu_msg(IGD_HOST_APP_MOD_QUEUE, &info, sizeof(info), mid, sizeof(mid));
	} else {
		i = mu_msg(IGD_APP_MOD_QUEUE, &info, sizeof(info), mid, sizeof(mid));
	}
	if (i < 0) {
		CGI_PRINTF("mac:%s, ret:%d\n", pmac ? pmac : "", i);
		return CGI_ERR_FAIL;
	}
	if (info.act != NLK_ACTION_DUMP)
		return 0;

	t_arr = json_object_new_array();
	for (i = 0; i < L7_MID_MX; i++) {
		if (!mid[i])
			continue;
		t_obj = json_object_new_int(mid[i]);
		json_object_array_add(t_arr, t_obj);
	}
	json_object_object_add(response, "queue", t_arr);
	return 0;
}

int cgi_up_ready_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	CHECK_LOGIN;

	return 0;
}

/*interface module cgi*/
int pro_net_wan_set_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int uid = 0;
	int mode = 0;
	char macstr[18];
	struct in_addr ip;
	struct host_info info;
	unsigned char mac[ETH_ALEN];
	struct json_object *obj = NULL;
	struct iface_conf config, oldcfg, cfgbak;
	char *user_s = NULL, *passwd_s = NULL, *status = NULL, *dns1_s = NULL;
	char *mode_s = NULL,*mac_s = NULL, *ip_s = NULL, *clone = NULL;
	char *mask_s = NULL, *gw_s = NULL, *dns_s = NULL, *mtu_s = NULL;
	char *channel_s = NULL, *ssid_s = NULL, *security_s = NULL, *key_s = NULL;
	
	if(connection_is_set(con)) {
		status = uci_getval("qos_rule", "status", "status");
		if (NULL == status)
			return CGI_ERR_FAIL;
		if (status[0] == '1') {
			CHECK_LOGIN;
		}
		clone = con_value_get(con, "clone");
		if (clone) {
			switch(atoi(clone)) {
			case 1:
				mac_s = con_value_get(con, "mac");
				if (NULL == mac_s || checkmac(mac_s))
					return CGI_ERR_INPUT;
				parse_mac(mac_s, mac);
				break;
			case 2:
				inet_aton(con->ip_from, &ip);
				if (dump_host_info(ip, &info)) 
					return CGI_ERR_FAIL;
				if (info.is_wifi)
					return CGI_ERR_NOSUPPORT;
				memcpy(mac, info.mac, ETH_ALEN);
				break;
			case 3:
				if (mu_msg(IF_MOD_PARAM_SHOW, &uid, sizeof(uid), &config, sizeof(config)))
					return CGI_ERR_FAIL;
				memcpy(mac, config.isp.mac, ETH_ALEN);
				mode = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];
				mode += 1;
				mac[2] = (mode >> 24) & 0xff;
				mac[3] = (mode >> 16) & 0xff;
				mac[4] = (mode >> 8) & 0xff;
				mac[5] = mode & 0xff;
				break;
			default:
				return CGI_ERR_INPUT;
			}
		} else {
			mac_s = con_value_get(con, "mac");
			if (NULL == mac_s || checkmac(mac_s))
				return CGI_ERR_INPUT;
			parse_mac(mac_s, mac);
		}
		mode_s = con_value_get(con, "mode");
		if (NULL == mode_s)
			return CGI_ERR_INPUT;
		mode = atoi(mode_s);
		if (mode < MODE_DHCP || mode > MODE_WISP)
			return CGI_ERR_INPUT;
		uid = 1;
		memset(&oldcfg, 0x0, sizeof(oldcfg));
		if (mu_msg(IF_MOD_PARAM_SHOW, &uid, sizeof(uid), &oldcfg, sizeof(oldcfg)))
			return CGI_ERR_FAIL;
		memset(&cfgbak, 0x0, sizeof(cfgbak));
		if (mu_msg(IF_MOD_PARAM_BAK, &uid, sizeof(uid), &cfgbak, sizeof(cfgbak)))
			return CGI_ERR_FAIL;
		memset(&config, 0x0, sizeof(config));
		config.uid = 1;
		switch (mode) {
		case MODE_DHCP:
			config.mode = MODE_DHCP;
			mtu_s = con_value_get(con, "mtu");
			if (mtu_s)
				config.isp.mtu = atoi(mtu_s);
			else if (oldcfg.mode == MODE_DHCP && oldcfg.isp.mtu > 0)
				config.isp.mtu = oldcfg.isp.mtu;
			else if (cfgbak.mode == MODE_DHCP && cfgbak.isp.mtu > 0)
				config.isp.mtu = cfgbak.isp.mtu;
			else
				config.isp.mtu = 1500;
			if (config.isp.mtu > 1500 || config.isp.mtu < 1000)
				return CGI_ERR_INPUT;
			dns_s = con_value_get(con, "dns");
			if (dns_s) {
				if (strlen(dns_s) > 0 && checkip(dns_s))
					return CGI_ERR_INPUT;
				inet_aton(dns_s, &config.isp.dns[0]);
			} else if (oldcfg.mode == MODE_DHCP)
				config.isp.dns[0].s_addr = oldcfg.isp.dns[0].s_addr;
			else if (cfgbak.mode == MODE_DHCP)
				config.isp.dns[0].s_addr = cfgbak.isp.dns[0].s_addr;
			dns1_s = con_value_get(con, "dns1");
			if (dns1_s) {
				if (strlen(dns1_s) > 0 && checkip(dns1_s))
					return CGI_ERR_INPUT;
				inet_aton(dns1_s, &config.isp.dns[1]);
			} else if (oldcfg.mode == MODE_DHCP)
				config.isp.dns[1].s_addr = oldcfg.isp.dns[1].s_addr;
			else if (cfgbak.mode == MODE_DHCP)
				config.isp.dns[1].s_addr = cfgbak.isp.dns[1].s_addr;
			memcpy(config.isp.mac, mac, ETH_ALEN);
			break;
		case MODE_PPPOE:
			user_s = con_value_get(con, "user");
			passwd_s = con_value_get(con, "passwd");
			if (NULL == user_s || NULL == passwd_s)
				return CGI_ERR_INPUT;
			if (!strlen(user_s) || (strlen(user_s) > sizeof(config.isp.pppoe.user) - 1))
				return CGI_ERR_INPUT;
			if (!strlen(passwd_s) || (strlen(passwd_s) > sizeof(config.isp.pppoe.passwd) - 1))
				return CGI_ERR_INPUT;
			mtu_s = con_value_get(con, "mtu");
			if (mtu_s)
				config.isp.mtu = atoi(mtu_s);
			else if (oldcfg.mode == MODE_PPPOE && oldcfg.isp.mtu > 0)
				config.isp.mtu = oldcfg.isp.mtu;
			else if (cfgbak.mode == MODE_PPPOE && cfgbak.isp.mtu > 0)
				config.isp.mtu = cfgbak.isp.mtu;
			else
				config.isp.mtu = 1480;
			if (config.isp.mtu > 1492 || config.isp.mtu < 1000)
				return CGI_ERR_INPUT;
			dns_s = con_value_get(con, "dns");
			if (dns_s) {
				if (strlen(dns_s) > 0 && checkip(dns_s))
					return CGI_ERR_INPUT;
				inet_aton(dns_s, &config.isp.dns[0]);
			} else if (oldcfg.mode == MODE_PPPOE)
				config.isp.dns[0].s_addr = oldcfg.isp.dns[0].s_addr;
			else if (cfgbak.mode == MODE_PPPOE)
				config.isp.dns[0].s_addr = cfgbak.isp.dns[0].s_addr;
			dns1_s = con_value_get(con, "dns1");
			if (dns1_s) {
				if (strlen(dns1_s) > 0 && checkip(dns1_s))
					return CGI_ERR_INPUT;
				inet_aton(dns1_s, &config.isp.dns[1]);
			} else if (oldcfg.mode == MODE_PPPOE)
				config.isp.dns[1].s_addr = oldcfg.isp.dns[1].s_addr;
			else if (cfgbak.mode == MODE_PPPOE)
				config.isp.dns[1].s_addr = cfgbak.isp.dns[1].s_addr;
			config.mode = MODE_PPPOE;
			arr_strcpy(config.isp.pppoe.user, user_s);
			arr_strcpy(config.isp.pppoe.passwd, passwd_s);
			memcpy(config.isp.mac, mac, ETH_ALEN);
			break;
		case MODE_STATIC:
			ip_s = con_value_get(con, "ip");
			gw_s = con_value_get(con, "gw");
			mask_s = con_value_get(con, "mask");
			if (ip_s == NULL || gw_s == NULL || mask_s == NULL)
				return CGI_ERR_INPUT;
			if (checkip(ip_s) || checkip(gw_s) || checkip(mask_s))
				return CGI_ERR_INPUT;
			mtu_s = con_value_get(con, "mtu");
			if (mtu_s)
				config.isp.mtu = atoi(mtu_s);
			else if (oldcfg.mode == MODE_STATIC && oldcfg.isp.mtu > 0)
				config.isp.mtu = oldcfg.isp.mtu;
			else if (cfgbak.mode == MODE_STATIC && cfgbak.isp.mtu > 0)
				config.isp.mtu = cfgbak.isp.mtu;
			else
				config.isp.mtu = 1500;
			if (config.isp.mtu > 1500 || config.isp.mtu < 1000)
				return CGI_ERR_INPUT;
			dns_s = con_value_get(con, "dns");
			if (dns_s) {
				if (strlen(dns_s) > 0 && checkip(dns_s))
					return CGI_ERR_INPUT;
				inet_aton(dns_s, &config.isp.dns[0]);
			} else if (oldcfg.mode == MODE_STATIC)
				config.isp.dns[0].s_addr = oldcfg.isp.dns[0].s_addr;
			else if (cfgbak.mode == MODE_STATIC)
				config.isp.dns[0].s_addr = cfgbak.isp.dns[0].s_addr;
			dns1_s = con_value_get(con, "dns1");
			if (dns1_s) {
				if (strlen(dns1_s) > 0 && checkip(dns1_s))
					return CGI_ERR_INPUT;
				inet_aton(dns1_s, &config.isp.dns[1]);
			} else if (oldcfg.mode == MODE_STATIC)
				config.isp.dns[1].s_addr = oldcfg.isp.dns[1].s_addr;
			else if (cfgbak.mode == MODE_STATIC)
				config.isp.dns[1].s_addr = cfgbak.isp.dns[1].s_addr;	
			config.mode = MODE_STATIC;
			inet_aton(ip_s, &config.isp.statip.ip);
			inet_aton(mask_s, &config.isp.statip.mask);
			inet_aton(gw_s, &config.isp.statip.gw);
			if (checkmask(ntohl(config.isp.statip.mask.s_addr)))
				return CGI_ERR_INPUT;
			if (ip_check2(config.isp.statip.ip.s_addr, config.isp.statip.gw.s_addr, config.isp.statip.mask.s_addr))
				return CGI_ERR_INPUT;
			memcpy(config.isp.mac, mac, ETH_ALEN);
			break;
		case MODE_WISP:
			channel_s = con_value_get(con, "channel");
			ssid_s = con_value_get(con, "ssid");
			security_s = con_value_get(con, "security");
			key_s = con_value_get(con, "key");
			key_s = (key_s && strlen(key_s)) ? key_s : "NONE";
			if (channel_s == NULL || ssid_s == NULL || security_s == NULL || key_s == NULL)
				return CGI_ERR_INPUT;
			if (atoi(channel_s) <= 0)
				return CGI_ERR_INPUT;
			if (!strlen(ssid_s) || (strlen(ssid_s) > sizeof(config.isp.wisp.ssid) - 1))
				return CGI_ERR_INPUT;
			if (!strlen(security_s) || (strlen(security_s) > sizeof(config.isp.wisp.security) - 1))
				return CGI_ERR_INPUT;
			if (!strlen(key_s) || (strlen(key_s) > sizeof(config.isp.wisp.key) - 1))
				return CGI_ERR_INPUT;
			mtu_s = con_value_get(con, "mtu");
			if (mtu_s)
				config.isp.mtu = atoi(mtu_s);
			else if (oldcfg.mode == MODE_WISP && oldcfg.isp.mtu > 0)
				config.isp.mtu = oldcfg.isp.mtu;
			else if (cfgbak.mode == MODE_WISP && cfgbak.isp.mtu > 0)
				config.isp.mtu = cfgbak.isp.mtu;
			else
				config.isp.mtu = 1500;
			if (config.isp.mtu > 1500 || config.isp.mtu < 1000)
				return CGI_ERR_INPUT;
			dns_s = con_value_get(con, "dns");
			if (dns_s) {
				if (strlen(dns_s) > 0 && checkip(dns_s))
					return CGI_ERR_INPUT;
				inet_aton(dns_s, &config.isp.dns[0]);
			} else if (oldcfg.mode == MODE_WISP)
				config.isp.dns[0].s_addr = oldcfg.isp.dns[0].s_addr;
			else if (cfgbak.mode == MODE_WISP)
				config.isp.dns[0].s_addr = cfgbak.isp.dns[0].s_addr;
			dns1_s = con_value_get(con, "dns1");
			if (dns1_s) {
				if (strlen(dns1_s) > 0 && checkip(dns1_s))
					return CGI_ERR_INPUT;
				inet_aton(dns1_s, &config.isp.dns[1]);
			} else if (oldcfg.mode == MODE_WISP)
				config.isp.dns[1].s_addr = oldcfg.isp.dns[1].s_addr;
			else if (cfgbak.mode == MODE_WISP)
				config.isp.dns[1].s_addr = cfgbak.isp.dns[1].s_addr;
			config.mode = MODE_WISP;
			config.isp.wisp.channel = atoi(channel_s);
			arr_strcpy(config.isp.wisp.ssid, ssid_s);
			arr_strcpy(config.isp.wisp.key, key_s);
			arr_strcpy(config.isp.wisp.security, security_s);
			memcpy(config.isp.mac, mac, ETH_ALEN);
			break;
		default:
			return CGI_ERR_INPUT;
		}
		if (mu_msg(IF_MOD_PARAM_SET, &config, sizeof(config), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	} else {
		memset(&config, 0x0, sizeof(config));
		uid = 1;
		if (mu_msg(IF_MOD_PARAM_SHOW, &uid, sizeof(uid), &config, sizeof(config)))
			return CGI_ERR_FAIL;
		switch (config.mode) {
		case MODE_DHCP:
			break;
		case MODE_PPPOE:
			obj = json_object_new_string(config.isp.pppoe.user);
			json_object_object_add(response, "user", obj);
			obj = json_object_new_string(config.isp.pppoe.passwd);
			json_object_object_add(response, "passwd", obj);
			break;
		case MODE_STATIC:
			obj = json_object_new_string(inet_ntoa(config.isp.statip.ip));
			json_object_object_add(response, "ip", obj);
			obj = json_object_new_string(inet_ntoa(config.isp.statip.mask));
			json_object_object_add(response, "mask", obj);
			obj = json_object_new_string(inet_ntoa(config.isp.statip.gw));
			json_object_object_add(response, "gw", obj);
			break;
		case MODE_WISP:
			obj = json_object_new_int(config.isp.wisp.channel);
			json_object_object_add(response, "channel", obj);
			obj = json_object_new_string(config.isp.wisp.ssid);
			json_object_object_add(response, "ssid", obj);
			obj = json_object_new_string(config.isp.wisp.security);
			json_object_object_add(response, "security", obj);
			obj = json_object_new_string(config.isp.wisp.key);
			json_object_object_add(response, "key", obj);
			if (!mu_msg(IF_MOD_PARAM_BAK, &uid, sizeof(uid), &cfgbak, sizeof(cfgbak))) {
				switch (cfgbak.mode) {
				case MODE_PPPOE:
					obj = json_object_new_string(cfgbak.isp.pppoe.user);
					json_object_object_add(response, "user", obj);
					obj = json_object_new_string(cfgbak.isp.pppoe.passwd);
					json_object_object_add(response, "passwd", obj);
					break;
				case MODE_STATIC:
					obj = json_object_new_string(inet_ntoa(cfgbak.isp.statip.ip));
					json_object_object_add(response, "ip", obj);
					obj = json_object_new_string(inet_ntoa(cfgbak.isp.statip.mask));
					json_object_object_add(response, "mask", obj);
					obj = json_object_new_string(inet_ntoa(cfgbak.isp.statip.gw));
					json_object_object_add(response, "gw", obj);
					break;
				case MODE_DHCP:
					break;
				default:
					break;
				}
				obj = json_object_new_int(cfgbak.mode);
				json_object_object_add(response, "oldmode", obj);
				obj = json_object_new_int(cfgbak.isp.mtu);
				json_object_object_add(response, "oldmtu", obj);
				if (cfgbak.isp.dns[0].s_addr)
					obj = json_object_new_string(inet_ntoa(cfgbak.isp.dns[0]));
				else 
					obj = json_object_new_string("");
				json_object_object_add(response, "olddns", obj);
				if (cfgbak.isp.dns[1].s_addr)
					obj = json_object_new_string(inet_ntoa(cfgbak.isp.dns[1]));
				else 
					obj = json_object_new_string("");
				json_object_object_add(response, "olddns1", obj);
			}
			break;
		default:
			return CGI_ERR_FAIL;
		}
		obj = json_object_new_int(config.mode);
		json_object_object_add(response, "mode", obj);
		snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_SPLIT(config.isp.mac));
		obj = json_object_new_string(macstr);
		json_object_object_add(response, "mac", obj);
		obj = json_object_new_int(config.isp.mtu);
		json_object_object_add(response, "mtu", obj);
		if (config.isp.dns[0].s_addr)
			obj = json_object_new_string(inet_ntoa(config.isp.dns[0]));
		else 
			obj = json_object_new_string("");
		json_object_object_add(response, "dns", obj);
		if (config.isp.dns[1].s_addr)
			obj = json_object_new_string(inet_ntoa(config.isp.dns[1]));
		else 
			obj = json_object_new_string("");
		json_object_object_add(response, "dns1", obj);
		/*add host mac and raw mac show*/
		inet_aton(con->ip_from, &ip);
		if (!dump_host_info(ip, &info) && !info.is_wifi) {
			snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_SPLIT(info.mac));
			obj = json_object_new_string(macstr);
		} else {
			obj = json_object_new_string("");
		}
		json_object_object_add(response, "hostmac", obj);
		uid = 0;
		if (mu_msg(IF_MOD_PARAM_SHOW, &uid, sizeof(uid), &config, sizeof(config))) {
			obj = json_object_new_string("");
		} else {
			memcpy(mac, config.isp.mac, ETH_ALEN);
			mode = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];
			mode += 1;
			mac[2] = (mode >> 24) & 0xff;
			mac[3] = (mode >> 16) & 0xff;
			mac[4] = (mode >> 8) & 0xff;
			mac[5] = mode & 0xff;
			snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_SPLIT(mac));
			obj = json_object_new_string(macstr);
		}
		json_object_object_add(response, "rawmac", obj);
		return 0;
	}
}



int pro_net_wan_info_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int uid = 1;
	char macstr[18];
	struct iface_info info;
	struct json_object* obj;

	if (connection_is_set(con))
		return CGI_ERR_INPUT;
	memset(&info, 0x0, sizeof(info));
	if (mu_msg(IF_MOD_IFACE_INFO, &uid, sizeof(uid), &info, sizeof(info)))
		return CGI_ERR_FAIL;
	obj = json_object_new_int(info.net);
	json_object_object_add(response, "connected", obj);
	obj = json_object_new_int(info.link);
	json_object_object_add(response, "link", obj);
	obj = json_object_new_int(info.mode);
	json_object_object_add(response, "mode", obj);
	obj = json_object_new_string(inet_ntoa(info.ip));
	json_object_object_add(response, "ip", obj);
	obj = json_object_new_string(inet_ntoa(info.mask));
	json_object_object_add(response, "mask", obj);
	obj= json_object_new_string(inet_ntoa(info.gw));
	json_object_object_add(response, "gateway", obj);
	obj= json_object_new_string(inet_ntoa(info.dns[0]));
	json_object_object_add(response, "DNS1", obj);
	obj= json_object_new_string(inet_ntoa(info.dns[1]));
	json_object_object_add(response, "DNS2", obj);
	snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_SPLIT(info.mac));
	obj= json_object_new_string(macstr);
	json_object_object_add(response, "mac", obj);
	obj= json_object_new_int(info.mtu);
	json_object_object_add(response, "mtu", obj);
	obj= json_object_new_int(info.err);
	json_object_object_add(response, "reason", obj);
	return 0;
}

int pro_net_wan_account_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int option = 0;
	struct account_info info;
	struct json_object *obj;

	if (connection_is_set(con)) {
		if (mu_msg(IF_MOD_GET_ACCOUNT, &option, sizeof(int), &info, sizeof(info)))
			return CGI_ERR_FAIL;
		return 0;
	}

	option = 1;
	if (mu_msg(IF_MOD_GET_ACCOUNT, &option, sizeof(int), &info, sizeof(info)))
		return CGI_ERR_ACCOUNT_NOTREADY;
	obj= json_object_new_string(info.passwd);
	json_object_object_add(response, "passwd", obj);
	obj= json_object_new_string(info.user);
	json_object_object_add(response, "account", obj);
	return 0; 
}

int pro_net_wan_detect_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	struct json_object* obj;
	struct detect_info info;

	memset(&info, 0x0, sizeof(info));
	if (mu_msg(IF_MOD_WAN_DETECT, NULL, 0, &info, sizeof(info)))
		return CGI_ERR_FAIL;
	obj = json_object_new_int(info.link);
	json_object_object_add(response, "wan_link", obj);
	obj = json_object_new_int(info.mode);
	json_object_object_add(response, "mode", obj);
	obj = json_object_new_int(info.net);
	json_object_object_add(response, "connected", obj);
	return 0;
}

/*wifi module cgi*/
int pro_net_wifi_ap_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	struct wifi_conf config;
	struct json_object* obj;
	char *ssid = NULL, *passwd = NULL;
	char *channel = NULL, *hidden = NULL, *bw = NULL;
	struct nlk_sys_msg nlk;

	if (connection_is_set(con)) {
		CHECK_LOGIN;
		ssid = con_value_get(con, "ssid");
		passwd = con_value_get(con, "passwd");
		channel = con_value_get(con, "channel");
		hidden = con_value_get(con, "hidden");
		bw = con_value_get(con, "bw");
		if (NULL == ssid ||(strlen(ssid) <= 0) || (strlen(ssid) > 31))
			return CGI_ERR_INPUT;
		memset(&config, 0x0, sizeof(config));
		if (mu_msg(WIFI_MOD_GET_CONF, NULL, 0, &config, sizeof(config)))
			return CGI_ERR_FAIL;
		if (channel) {
			config.channel = atoi(channel);
			if (config.channel < 0 || config.channel > 13)
				return CGI_ERR_INPUT;
		}
		if (hidden) {
			config.hidssid = atoi(hidden);
			if (config.hidssid != 0 && config.hidssid != 1)
				return CGI_ERR_INPUT;
		}
		if (bw) {
			config.htbw = atoi(bw);
			if (config.htbw != 0 && config.htbw != 1)
				return CGI_ERR_INPUT;
		}
		arr_strcpy(config.ssid, ssid);
		if (passwd == NULL ||!strlen(passwd))
			arr_strcpy(config.encryption, "none");
		else {
			if (!strcmp(config.encryption, "none"))
				arr_strcpy(config.encryption, "psk2+ccmp");
			arr_strcpy(config.key, passwd);
		}

		memset(&nlk, 0x0, sizeof(nlk));
		nlk.comm.gid = NLKMSG_GRP_SYS;
		nlk.comm.mid = SYS_GRP_MID_WIFI;
		nlk.msg.wifi.type = 2;
		nlk_event_send(NLKMSG_GRP_SYS, &nlk, sizeof(nlk));

		if (mu_msg(WIFI_MOD_SET_AP, &config, sizeof(config), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}

	if (mu_msg(WIFI_MOD_GET_CONF, NULL, 0, &config, sizeof(config)))
		return CGI_ERR_FAIL;
	obj = json_object_new_string(config.ssid);
	json_object_object_add(response, "ssid", obj);
	obj = json_object_new_int(config.hidssid);
	json_object_object_add(response, "hidden", obj);
	obj = json_object_new_int(config.channel);
	json_object_object_add(response, "channel", obj);
	obj = json_object_new_int(config.htbw);
	json_object_object_add(response, "bw", obj);
	if (!strncmp(config.encryption, "none", 4)) {
		obj= json_object_new_int(0);
		json_object_object_add(response, "security", obj);
		obj= json_object_new_string("");
		json_object_object_add(response, "passwd", obj);
		return 0; 
	}
	
	if (con->login == 1) {
		obj= json_object_new_int(1);
		json_object_object_add(response, "security", obj);
		obj= json_object_new_string(config.key);
		json_object_object_add(response, "passwd", obj);
	} else {
		obj= json_object_new_int(0);
		json_object_object_add(response, "security", obj);
		obj= json_object_new_string("");
		json_object_object_add(response, "passwd", obj);
	}
	return 0;
}

int pro_net_wifi_lt_handler(server_t *srv, connection_t *con, struct json_object *response)
{
	char *ptr = NULL;
	unsigned char i = 0;
	int start, end, now;
	struct tm *tm = get_tm();
	struct wifi_conf config;
	struct time_comm *t = &config.tm;
	struct json_object* obj, *week, *day;
	
	memset(&config, 0, sizeof(config));
	if (connection_is_set(con)) {
		CHECK_LOGIN;
		if (mu_msg(WIFI_MOD_GET_CONF, NULL, 0, &config, sizeof(config)))
			return CGI_ERR_FAIL;
		memset(&config.tm, 0x0, sizeof(config.tm));
		ptr = con_value_get(con, "enable");
		if (!ptr) {
			CGI_PRINTF("input err, %p\n", ptr);
			return CGI_ERR_INPUT;
		}
		config.enable = atoi(ptr);
		ptr = con_value_get(con, "time_on");
		if (!ptr) {
			CGI_PRINTF("input err, %p\n", ptr);
			return CGI_ERR_INPUT;
		}
		config.time_on = atoi(ptr);
		ptr = con_value_get(con, "week");
		if (!ptr) {
			CGI_PRINTF("input err, %p\n", ptr);
			return CGI_ERR_INPUT;
		}
		for (i = 0; i < 7; i++) {
			if (*(ptr + i) == '\0') {
				CGI_PRINTF("week err\n");
				break;
			}
			if (*(ptr + i) == '1')
				CGI_BIT_SET(t->day_flags, i);
		}
		CON_GET_CHECK_INT(ptr, con, t->start_hour, "sh", 23);
		CON_GET_CHECK_INT(ptr, con, t->start_min, "sm", 59);
		CON_GET_CHECK_INT(ptr, con, t->end_hour, "eh", 23);
		CON_GET_CHECK_INT(ptr, con, t->end_min, "em", 59);
		if (!t->day_flags && tm && config.time_on) {
			start = t->start_hour*60 + t->start_min;
			end = t->end_hour*60 + t->end_min;
			now = tm->tm_hour*60 + tm->tm_min;
			if ((start < end) && (now > end))
				return CGI_ERR_TIMEOUT;
		}
		if (t->day_flags)
			t->loop = 1;
		else
			CGI_BIT_SET(t->day_flags, tm->tm_wday);
		if (mu_msg(WIFI_MOD_SET_TIME, &config, sizeof(config), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}

	if (mu_msg(WIFI_MOD_GET_CONF, NULL, 0, &config, sizeof(config)))
		return CGI_ERR_FAIL;
	obj = json_object_new_int(config.enable);
	json_object_object_add(response, "enable", obj);
	obj = json_object_new_int(config.time_on);
	json_object_object_add(response, "time_on", obj);
	week = json_object_new_array();
	if (t->loop) {
		for (i = 0; i < 7; i++) {
			if (!CGI_BIT_TEST(t->day_flags, i))
				continue;
			day = json_object_new_int(i);
			json_object_array_add(week, day);
		}
	}
	json_object_object_add(response, "week", week);
	obj = json_object_new_int(t->start_hour);
	json_object_object_add(response, "sh", obj);

	obj = json_object_new_int(t->start_min);
	json_object_object_add(response, "sm", obj);

	obj = json_object_new_int(t->end_hour);
	json_object_object_add(response, "eh", obj);

	obj = json_object_new_int(t->end_min);
	json_object_object_add(response, "em", obj);
	return 0;
}

int pro_net_wifi_vap_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int enable = 0;
	char *vssid_s = NULL;
	char *enable_s = NULL;
	struct wifi_conf config;
	struct json_object* obj;

	memset(&config, 0x0, sizeof(config));
	if (connection_is_set(con)) {
		CHECK_LOGIN;
		enable_s = con_value_get(con, "enable");
		vssid_s = con_value_get(con, "vssid");
		if (enable_s == NULL)
			return CGI_ERR_INPUT; 
		if (mu_msg(WIFI_MOD_GET_CONF, NULL, 0, &config, sizeof(config)))
			return CGI_ERR_FAIL;
		enable = atoi(enable_s);
		config.vap = enable;
		if (vssid_s)
			arr_strcpy(config.vssid, vssid_s);
		if (mu_msg(WIFI_MOD_SET_VAP, &config, sizeof(config), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}
	if (mu_msg(WIFI_MOD_GET_CONF, NULL, 0, &config, sizeof(config)))
		return CGI_ERR_FAIL;
	obj = json_object_new_int(config.vap);
	json_object_object_add(response, "enable", obj);
	obj = json_object_new_string(config.vssid);
	json_object_object_add(response, "vssid", obj);
	return 0;
}

int pro_net_wifi_vap_host_hander(server_t* srv, connection_t *con, struct json_object*response)
{
	int i = 0, nr = 0;
	char *mac_s = NULL;
	char *action_s = NULL;
	char macstr[18];
	unsigned char mac[ETH_ALEN];
	struct host_info info[HOST_MX];
	struct json_object *obj, *vhosts, *host;
	
	if (connection_is_set(con)) {
		CHECK_LOGIN;
		mac_s = con_value_get(con, "mac");
		action_s = con_value_get(con, "action");
		if (!action_s || !mac_s || checkmac(mac_s))
			return CGI_ERR_INPUT;
		parse_mac(mac_s, mac);
		if (!strncmp(action_s, "add", 3)) {
			if (mu_msg(WIFI_MOD_VAP_HOST_ADD, mac, sizeof(mac), NULL, 0))
				return CGI_ERR_FAIL;
		} else if (!strncmp(action_s, "del", 3)) {
			if (mu_msg(WIFI_MOD_VAP_HOST_DEL, mac, sizeof(mac), NULL, 0))
				return CGI_ERR_FAIL;
		} else
			return CGI_ERR_INPUT;
		return 0;
	}
	memset(&info, 0x0, sizeof(info));
	nr = mu_msg(WIFI_MOD_VAP_HOST_DUMP, NULL, 0, info, sizeof(struct host_info) * HOST_MX);
	if (nr < 0)
		return CGI_ERR_FAIL;
	obj = json_object_new_int(nr);
	json_object_object_add(response, "count", obj);
	vhosts = json_object_new_array();
	for (i = 0; i < nr; i++) {
		host = json_object_new_object();
		obj = json_object_new_int(info[i].vender);
		json_object_object_add(host, "vender", obj);
		obj = json_object_new_int(info[i].os_type);
		json_object_object_add(host, "ostype", obj);
		if (info[i].nick_name[0]) {
			obj = json_object_new_string(info[i].nick_name);
			json_object_object_add(host, "name", obj);
		} else {
			obj = json_object_new_string(info[i].name);
			json_object_object_add(host, "name", obj);
		}
		snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_SPLIT(info[i].mac));
		obj = json_object_new_string(macstr);
		json_object_object_add(host, "mac", obj);
		obj = json_object_new_int(info[i].pid);
		json_object_object_add(host, "wlist", obj);
		json_object_array_add(vhosts, host);
	}
	json_object_object_add(response, "guests", vhosts);
	return 0;
}

int pro_net_txrate_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int txrate;
	char *txrates = NULL;
	struct wifi_conf config;
	struct json_object* obj;
	
	if (connection_is_set(con)) {
		txrates = con_value_get(con, "txrate");
		if (txrates == NULL)
			return CGI_ERR_INPUT; 
		txrate = atoi(txrates);
		if (txrate > 100 || txrate < 10)
			return CGI_ERR_INPUT; 
		if (mu_msg(WIFI_MOD_SET_TXRATE, &txrate, sizeof(txrate), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}
	memset(&config, 0x0, sizeof(config));
	if (mu_msg(WIFI_MOD_GET_CONF, NULL, 0, &config, sizeof(config)))
		return CGI_ERR_FAIL;
	obj = json_object_new_int(config.txrate);
	json_object_object_add(response, "txrate", obj);
	return 0;
	
}

int pro_net_ap_list_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int i;
	int nr = 0;
	int scan = 1;
	struct json_object *obj;
	struct json_object *aps;
	struct json_object *ap;
	struct iwsurvey survey[IW_SUR_MX];
	
	if (connection_is_set(con))
		return CGI_ERR_FAIL;
	nr = mu_msg(WIFI_MOD_GET_SURVEY, &scan, sizeof(int), survey, sizeof(struct iwsurvey) * IW_SUR_MX);
	if (nr > 0)
		goto finish;
	usleep(5000000);
	scan = 0;
	nr = mu_msg(WIFI_MOD_GET_SURVEY, &scan, sizeof(int), survey, sizeof(struct iwsurvey) * IW_SUR_MX);
	if (nr <= 0)
		return CGI_ERR_FAIL;
finish:
	obj = json_object_new_int(nr);
	json_object_object_add(response, "count", obj);
	aps = json_object_new_array();
	for (i = 0; i < nr; i++) {
		ap = json_object_new_object();
		obj = json_object_new_int(survey[i].ch);
		json_object_object_add(ap, "channel", obj);
		obj = json_object_new_int(survey[i].signal);
		json_object_object_add(ap, "dbm", obj);
		obj = json_object_new_string(survey[i].ssid);
		json_object_object_add(ap, "ssid", obj);
		obj = json_object_new_string(survey[i].extch);
		json_object_object_add(ap, "extch", obj);
		obj = json_object_new_string(survey[i].security);
		json_object_object_add(ap, "security", obj);
		obj = json_object_new_string(survey[i].mode);
		json_object_object_add(ap, "mode", obj);
		obj = json_object_new_string(survey[i].bssid);
		json_object_object_add(ap, "bssid", obj);
		obj = json_object_new_int(survey[i].wps);
		json_object_object_add(ap, "wps", obj);
		json_object_array_add(aps,ap);
	}
	json_object_object_add(response, "aplist", aps);
	return 0;
}

int pro_net_channel_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int ret = 0;
	int channel = 0;
	char *channels = NULL;
	struct json_object* obj;

	if (connection_is_set(con)) {
		channels  = con_value_get(con, "channel");
		if (channels == NULL)
			return CGI_ERR_INPUT; 
		channel = atoi(channels);
		if (channel <= 0 || channel > 13)
			return CGI_ERR_INPUT;
		if (mu_msg(WIFI_MOD_SET_CHANNEL, &channel, sizeof(channel), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}
	//if wifi disable,return CGI_ERR_NONEXIST
	ret = mu_msg(WIFI_MOD_GET_CHANNEL, NULL, 0, &channel, sizeof(channel));
	if (ret > 0)
		return CGI_ERR_NOSUPPORT;
	else if (ret < 0)
		return CGI_ERR_FAIL;
	else if (channel <= 0 ||channel > 13)
		return CGI_ERR_FAIL;
	obj = json_object_new_int(channel);
	json_object_object_add(response, "channel", obj);
	return 0; 
}

/*dnsmasq/dhcp module cgi*/
int pro_net_dhcpd_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int reboot = 1;
	int uid = 0;
	struct iface_conf ifc;
	struct dhcp_conf config;
	struct json_object* obj;
	char *enable_s = NULL;
	char *ip_s = NULL, *mask_s = NULL;
	char *start_s = NULL, *end_s = NULL;
	
	if (connection_is_set(con)) {
		CHECK_LOGIN;
		start_s = con_value_get(con, "start");
		end_s = con_value_get(con, "end");
		mask_s = con_value_get(con, "mask");
		ip_s = con_value_get(con, "ip");
		enable_s = con_value_get(con, "enable");
		if (!ip_s || !mask_s || !start_s || !end_s)
			return CGI_ERR_INPUT;
		if (checkip(mask_s) || checkip(ip_s) || checkip(start_s) || checkip(end_s))
			return CGI_ERR_INPUT;
		memset(&ifc, 0x0, sizeof(ifc));
		if (mu_msg(IF_MOD_PARAM_SHOW, &uid, sizeof(uid), &ifc, sizeof(ifc)))
			return CGI_ERR_FAIL;
		if (enable_s)
			config.enable = atoi(enable_s);
		else
			config.enable = 1;
		inet_aton(ip_s, &config.ip);
		inet_aton(mask_s, &config.mask);
		inet_aton(start_s, &config.start);
		inet_aton(end_s, &config.end);
		if (checkmask(ntohl(config.mask.s_addr)))
			return CGI_ERR_INPUT;
		if (ntohl(config.end.s_addr) < ntohl(config.start.s_addr))
			return CGI_ERR_INPUT;
		if (ip_check2(config.ip.s_addr, config.start.s_addr, config.mask.s_addr))
			return CGI_ERR_INPUT;
		if (ip_check2(config.ip.s_addr, config.end.s_addr, config.mask.s_addr))
			return CGI_ERR_INPUT;
		if ((ifc.isp.statip.ip.s_addr != config.ip.s_addr) || (ifc.isp.statip.mask.s_addr != config.mask.s_addr)) {
			ifc.isp.statip.ip = config.ip;
			ifc.isp.statip.mask = config.mask;
			if (mu_msg(IF_MOD_PARAM_SET, &ifc,  sizeof(ifc), NULL, 0))
				return CGI_ERR_FAIL;
			if (mu_msg(UPNPD_SERVER_SET, NULL, 0, NULL, 0))
				return CGI_ERR_FAIL;
		}
		if (mu_msg(DNSMASQ_DHCP_SET, &config, sizeof(config), NULL, 0))
			return CGI_ERR_FAIL;	
		if (mu_msg(WIFI_MOD_DISCONNCT_ALL, NULL, 0, NULL, 0))
			return CGI_ERR_FAIL;
		obj= json_object_new_int(reboot);
		json_object_object_add(response, "reboot", obj);
		return 0;
	}
	if (mu_msg(DNSMASQ_DHCP_SHOW, NULL, 0, &config, sizeof(config)))
		return CGI_ERR_FAIL;
	obj= json_object_new_int(reboot);
	json_object_object_add(response, "reboot", obj);
	obj= json_object_new_string(inet_ntoa(config.start));
	json_object_object_add(response, "start", obj);
	obj= json_object_new_string(inet_ntoa(config.end));
	json_object_object_add(response, "end", obj);
	obj= json_object_new_string(inet_ntoa(config.ip));
	json_object_object_add(response, "ip", obj);
	obj= json_object_new_string(inet_ntoa(config.mask));
	json_object_object_add(response, "mask", obj);
	obj= json_object_new_int(config.enable);
	json_object_object_add(response, "enable", obj);
	return 0;
}

static void get_userid_md5(char *md5str)
{
	int i = 0;
	char str[128];
	unsigned char md5[64];
	unsigned int usrid;

	md5str[0] = 0;
	usrid = get_usrid();
	if (!usrid)
		return;

	snprintf(str, sizeof(str), "&*E#N@DER_$%u", usrid);
	get_md5_numbers((unsigned char *)str, md5, strlen(str));
	memset(str, 0x0, sizeof(str));
	for (i = 0; i < 16; i++)
		sprintf(&md5str[i*2], "%02X", md5[i]);	
}

static void get_login_account_md5(char *md5str)
{
	int i = 0;
	char str[128];
	unsigned char md5[64];
	struct sys_account info;

	md5str[0] = 0;
	if (mu_msg(SYSTME_MOD_GET_ACCOUNT, NULL, 0, &info, sizeof(info)))
		return;

	snprintf(str, sizeof(str), "%s%s", info.user, info.password);
	get_md5_numbers((unsigned char *)str, md5, strlen(str));
	memset(str, 0x0, sizeof(str));
	for (i = 0; i < 16; i++)
		sprintf(&md5str[i*2], "%02X", md5[i]);	
}


/*system module cgi*/
int pro_sys_login_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	struct in_addr addr;
	struct host_info info;
	struct nlk_sys_msg nlk;
	char *usrid, userid_md5[64], account_md5[64];
	
	if (!con->ip_from)
		return CGI_ERR_FAIL;

	usrid = con_value_get(con, "usrid");
	if (!usrid)
		return CGI_ERR_INPUT;

	get_userid_md5(userid_md5);
	get_login_account_md5(account_md5);

	if (strncasecmp(userid_md5, usrid, 32)
		&& strncasecmp(account_md5, usrid, 32)) {
		inet_aton(con->ip_from, &addr);
		if (!dump_host_info(addr, &info)) {
			memset(&nlk, 0x0, sizeof(nlk));
			nlk.comm.gid = NLKMSG_GRP_SYS;
			nlk.comm.mid = SYS_GRP_MID_ADMIN;
			memcpy(nlk.msg.admin.mac, info.mac, ETH_ALEN);
			nlk_event_send(NLKMSG_GRP_SYS, &nlk, sizeof(nlk));
		}
		return CGI_ERR_FAIL;
	}
	if (con->ip_from)
		server_list_add(con->ip_from, srv);
	return 0;
}

int pro_sys_setting_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	CHECK_LOGIN;
	int flag = SYSFLAG_RECOVER;
	char *action  = NULL, dataup[8];
	
	if (!connection_is_set(con))
		return CGI_ERR_INPUT; 
	action  = con_value_get(con, "action");
	if (!action)
		return CGI_ERR_FAIL;
	if (!strncmp(action, "default", 7)) {
		if (mu_msg(SYSTEM_MOD_SYS_DEFAULT, &flag, sizeof(int), NULL, 0))
			return CGI_ERR_FAIL;
		//send to cloud, unbind
		CC_PUSH2(dataup, 0, 8);
		CC_PUSH2(dataup, 2, CSO_REQ_ROUTER_RESET);
		CC_MSG_ADD(dataup, 8);
	} else if (!strncmp(action, "reboot", 6)) {
		if (mu_msg(SYSTEM_MOD_SYS_REBOOT, NULL, 0, NULL, 0))
			return CGI_ERR_FAIL;
	} else
		return CGI_ERR_INPUT; 
	return 0;
}

int pro_sys_deviceid_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	char tmpbuf[64];
	unsigned int id;
	struct json_object* obj;
	
	if (connection_is_set(con))
		return CGI_ERR_INPUT;
	id = get_devid();
	sprintf(tmpbuf, "%u", id);
	obj= json_object_new_string(tmpbuf);
	json_object_object_add(response, "deviceid", obj);
	return 0;
}

int pro_sys_sdevice_check_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int status = 0;
	struct json_object* obj;
	
	if (connection_is_set(con))
		return CGI_ERR_INPUT;
	if (mu_msg(SYSTEM_MOD_DEV_CHECK, NULL, 0, &status, sizeof(status)))
		return CGI_ERR_FAIL;
	obj= json_object_new_int(status);
	json_object_object_add(response, "config_status", obj);
	obj= json_object_new_string("CY_WiFi");
	json_object_object_add(response, "name", obj);
	return 0;
}

int pro_sys_firmup_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	CHECK_LOGIN;
	if (!connection_is_set(con))
		return CGI_ERR_INPUT;
	if (mu_msg(SYSTEM_MOD_FW_UPDATE, NULL, 0, NULL, 0))
		return CGI_ERR_FAIL;
	return 0;
}

static int check_firmare(void)
{
	struct image_header iheader;
	struct image_header dheader;
	char *dev;
	int devfd;
	int len;
	int imagefd;
	int ret = -1;

	dev = read_firmware("FIRMWARE");
	if (!dev)
		return -1;
	devfd = open(dev, O_RDONLY);
	if (devfd < 0)
		return -1;
	imagefd = open(FW_LOCAL_FILE, O_RDONLY);
	if (imagefd < 0) {
		close(devfd);
		return -1;
	}

	len = read(imagefd, (void *)&iheader, sizeof(iheader));
	if (len != sizeof(iheader))
		goto error;
	len = read(devfd, (void *)&dheader, sizeof(dheader));
	if (len != sizeof(dheader))
		goto error;
	if (strcmp((char *)iheader.ih_name, (char *)dheader.ih_name))
		goto error;
	ret = 0;
error:
	close(devfd);
	close(imagefd);
	return ret;
}

int pro_sys_local_firmup_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	CHECK_LOGIN;
	if (!connection_is_set(con))
		return CGI_ERR_INPUT;
	/*  check firmware valid */
	if (check_firmare())
		return CGI_ERR_FILE;
	if (mu_msg(SYSTME_MOD_LOCAL_UPDATE, NULL, 0, NULL, 0))
		return CGI_ERR_FAIL;
	return 0;
}

int pro_sys_upload_status_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int status = 0;
	struct json_object* obj;

	CHECK_LOGIN;
	if (connection_is_set(con))
		return CGI_ERR_INPUT;
	if (!access(FW_LOCAL_FILE, F_OK))
		status = 1;
	obj = json_object_new_int(status);
	json_object_object_add(response, "status", obj);
	return 0;
}

int pro_sys_firmstatus_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int nr = 0;
	char *flag = NULL;
	struct fw_info info;
	struct json_object* obj;
	char model[512];

	if (connection_is_set(con)) {
		flag = con_value_get(con, "flag");
		if((flag == NULL) || (strcmp(flag, "get") != 0))
			return CGI_ERR_INPUT;
		if (mu_msg(SYSTEM_MOD_FW_DOWNLOAD, NULL, 0, NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}

	while(1) {
		if (nr++ >= 16)
			break;
		memset(&info, 0x0, sizeof(info));
		mu_msg(SYSTEM_MOD_VERSION_CHECK, NULL, 0, &info, sizeof(struct fw_info));
		if (info.flag != FW_FLAG_NOFW)
			break;
		usleep(500000);
	}
	if (info.flag == FW_FLAG_FINISH)
		info.flag = FW_FLAG_NOFW;
	obj = json_object_new_int(info.flag);
	json_object_object_add(response, "status", obj);
	
	obj = json_object_new_int(info.speed);
	json_object_object_add(response, "speed", obj);
	
	obj = json_object_new_int(info.cur);
	json_object_object_add(response, "curl", obj);
	
	obj = json_object_new_int(atoi(info.size));
	json_object_object_add(response, "total", obj);
	
	obj = json_object_new_string(info.name);
	json_object_object_add(response, "name", obj);

	obj = json_object_new_string(info.time);
	json_object_object_add(response, "time", obj);
	
	obj = json_object_new_string(info.ver);
	json_object_object_add(response, "newfirm_version", obj);
	
	obj = json_object_new_string(info.desc);
	json_object_object_add(response, "manual", obj);

	obj = json_object_new_string(info.cur_ver);
	json_object_object_add(response, "localfirm_version", obj);

	nr = snprintf(model, sizeof(model), "%s", read_firmware("VENDOR"));
	nr += snprintf(&model[nr], sizeof(model) - nr, "_%s", read_firmware("PRODUCT"));
	obj = json_object_new_string(model);
	json_object_object_add(response, "model", obj);
	return 0;
}

int cgi_mtd_param_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	char value[32];
	char valname[32];
	char *valname_s;
	struct json_object* obj;
	
	CHECK_LOGIN;
	if (connection_is_set(con))
		return CGI_ERR_INPUT;
	valname_s = con_value_get(con, "valname");
	if (valname_s == NULL)
		return CGI_ERR_INPUT;
	arr_strcpy(valname, valname_s);
	if (mu_msg(SYSTME_MOD_MTD_PARAM, valname, sizeof(valname), value, sizeof(value)))
		return CGI_ERR_FAIL;
	obj = json_object_new_string(value);
	json_object_object_add(response, "value", obj);
	return 0;
}

/*QOS module cgi*/
int pro_net_qos_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	struct qos_conf config;
	struct json_object*obj;
	char *enable_s = NULL, *up_s = NULL, *down_s = NULL;
	
	if (connection_is_set(con)) {
		CHECK_LOGIN;
		enable_s = con_value_get(con, "enable");
		up_s = con_value_get(con, "up");
		down_s = con_value_get(con, "down");
		if (enable_s == NULL || up_s == NULL || down_s == NULL)
			return CGI_ERR_INPUT;

		memset(&config, 0, sizeof(config));
		if (enable_s[0] == '1')
			config.enable = 1;
		else if (enable_s[0] == '0')
			config.enable = 0;
		else
			return CGI_ERR_INPUT;
		config.up = atol(up_s);
		config.down = atol(down_s);
		
		if ((!config.up) || (!config.down)) {
			CGI_PRINTF("qos up/down err, %d, %d\n", config.up, config.down);
			return CGI_ERR_INPUT;
		}
		if (mu_msg(QOS_PARAM_SET, &config, sizeof(config), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}
	
	if (mu_msg(QOS_PARAM_SHOW, NULL, 0, &config, sizeof(config)))
		return CGI_ERR_FAIL;
	obj= json_object_new_int(config.enable);
	json_object_object_add(response, "enable", obj);
	obj= json_object_new_int(config.up);
	json_object_object_add(response, "up", obj);
	obj= json_object_new_int(config.down);
	json_object_object_add(response, "down", obj);
	return 0;
}

int pro_net_testspeed_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	char *ptr = NULL;
	struct tspeed_info info;
	struct json_object *obj = NULL;

	if (connection_is_set(con)) {
		if (mu_msg(QOS_TEST_SPEED, NULL, 0, NULL, 0))
			return CGI_ERR_FAIL;
	}

	ptr = con_value_get(con, "act");
	if (ptr && !strcmp(ptr, "off")) {
		if (mu_msg(QOS_TEST_BREAK, NULL, 0, NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	} else {
		if (mu_msg(QOS_SPEED_SHOW, NULL, 0, &info, sizeof(info)))
			return CGI_ERR_FAIL;
	}
	obj = json_object_new_int(info.flag);
	json_object_object_add(response, "flag", obj);
	if (info.flag == 1) {
		obj = json_object_new_int(info.ispeed);
		json_object_object_add(response, "ispeed", obj);
	} else if (info.flag == 2) {
		obj = json_object_new_int(info.down);
		json_object_object_add(response, "down_speed", obj);
		obj = json_object_new_int(info.up);
		json_object_object_add(response, "up_speed", obj);
		obj = json_object_new_int(info.delay);
		json_object_object_add(response, "delay", obj);
	}
	return 0;
}

/*url safe module cgi*/
int pro_net_rule_table_security_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int enable = 0;
	char *enable_s = NULL;
	struct json_object* obj;
	
	if (connection_is_set(con)) {
		CHECK_LOGIN;
		enable_s = con_value_get(con, "sercurity");
		if (!enable_s)
			return CGI_ERR_INPUT;
		enable = atoi(enable_s);
		if (mu_msg(URL_SAFE_MOD_SET_ENABLE, &enable, sizeof(enable), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}
	enable_s = uci_getval("qos_rule", "security", "status");
	if (!enable_s)
		return CGI_ERR_FAIL;
	enable = atoi(enable_s);
	obj= json_object_new_int(enable);
	json_object_object_add(response, "security", obj);
	free(enable_s);
	return 0;
}

int pro_net_url_safe_redirect_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	struct urlsafe_trust info; 
	char* ip = con_value_get(con,"ip");
	char* url = con_value_get(con,"url");

	if (NULL == ip || NULL == url)
		return CGI_ERR_INPUT;
	memset(&info, 0x0, sizeof(info));
	if (checkip(ip) || !strlen(url))
		return CGI_ERR_INPUT;
	inet_aton(ip, &info.ip);
	__arr_strcpy_end(info.url, (unsigned char *)url, sizeof(info.url), '\0');
	if (mu_msg(URL_SAFE_MOD_SET_IP, &info, sizeof(info), NULL, 0))
		return CGI_ERR_FAIL;
	return 0;
}

int cgi_net_advert_handler(server_t* srv, connection_t *con, struct json_object*response)
{
	int enable;
	char *enable_s = NULL;
	struct json_object* obj;
	
	if (connection_is_set(con)) {
		CHECK_LOGIN;
		enable_s = con_value_get(con, "enable");
		if (!enable_s)
			return CGI_ERR_INPUT;
		enable = atoi(enable_s);
		if (mu_msg(ADVERT_MOD_ACTION, &enable, sizeof(enable), NULL, 0))
			return CGI_ERR_FAIL;
		return 0;
	}
	if (mu_msg(ADVERT_MOD_DUMP, NULL, 0, &enable, sizeof(enable)))
		return CGI_ERR_FAIL;
	obj= json_object_new_int(enable);
	json_object_object_add(response, "enable", obj);
	return 0;
}


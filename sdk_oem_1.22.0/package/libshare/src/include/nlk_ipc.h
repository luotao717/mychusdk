#ifndef _NLK_IPC_H_
#define _NLK_IPC_H_
#include "igd_share.h"
#include "nlk_msg_c.h"

struct http_rule_api {
	struct inet_l3 l3;
	unsigned long flags;
	unsigned short mid;
	unsigned short period;
	unsigned short times;
	short prio;
	union {
		struct url_rd rd;
		char data[512];
	};
};

extern int get_if_statistics(int type, struct if_statistics *stat);
extern unsigned int get_devid(void);
extern unsigned int get_usrid(void);
extern int set_usrid(unsigned int usrid);
extern int del_group(int mid, int id);
//extern int clean_group(int mid);
extern struct nlk_dump_rule *dump_group(int mid, int *nr);
extern int mod_user_grp_by_mac(int id, char *name, int nr, char (*mac)[ETH_ALEN]);
extern int mod_user_grp_by_ip(int id, char *name, int nr, struct ip_range *ip);
extern int mod_url_grp(int id, char *name, int nr, char (*url)[IGD_URL_LEN]);
extern int mod_dns_grp(int id, char *name, int nr, char (*dns)[IGD_DNS_LEN]);
extern int add_url_grp(char *name, int nr, char (*url)[IGD_URL_LEN]);
extern int add_url_grp2(char *name, int nr, struct inet_url *url);
extern int add_dns_grp(char *name, int nr, char (*dns)[IGD_DNS_LEN]);
extern int add_user_grp_by_mac(char *name, int nr, unsigned char (*mac)[ETH_ALEN]);
extern int add_user_grp_by_ip(char *name, int nr, struct ip_range *ip);
extern int add_filter_rule(int mid, int prio, struct nf_rule_info *info);
extern int del_filter_rule(int mid, int id);
extern int mod_filter_rule(int mid, int id, int prio, struct nf_rule_info *info);
//extern int clean_filter_rule(int mid);
extern struct host_info *dump_host_alive(int *nr);
extern int dump_host_info(struct in_addr addr, struct host_info *info);
extern struct app_conn_info *dump_host_app(struct in_addr addr, int *nr);
extern struct conn_info *dump_host_conn(struct in_addr addr, int *nr);
extern int register_app_filter(struct inet_host *host, struct app_filter_rule *app);
extern int unregister_app_filter(int id);
extern int register_app_rate_limit(struct inet_host *host,
	       	int mid, int id, struct rate_entry *rate);
extern int register_app_qos(struct inet_host *host, int queue, int nr, int *app);
extern int unregister_app_rate_limit(int id);
extern int unregister_app_qos(int id);
extern int get_link_status(unsigned long *bitmap);
extern int igd_safe_read(int fd, unsigned char *dst, int len);
extern int igd_safe_write(int fd, unsigned char *src, int len);
extern int dump_flash_uid(unsigned char *buf);
extern int read_mac(unsigned char *mac);
extern void console_printf(const char *fmt, ...);
extern int register_http_rule(struct inet_host *host, struct http_rule_api *arg);
extern int unregister_http_rule(int mid, int id);
extern void igd_log(char *file, const char *fmt, ...);
extern int igd_aes_dencrypt(const char *path, unsigned char *out, int len, unsigned char *pwd);
extern int igd_aes_encrypt(const char *path, unsigned char *out, int len, unsigned char *pwd);
extern int set_dev_vlan(char *devname, uint16_t vlan_mask);
extern int set_dev_uid(char *devname, int uid);
extern int set_sysflag(unsigned char bit, unsigned char fg);
extern int igd_time_cmp(struct tm *t, struct time_comm *m);
extern long sys_uptime(void);
extern struct tm *get_tm(void);
extern int set_gpio_led(int action);
extern int set_wifi_led(int action);
extern int set_port_link(int pid, int action);
extern char *read_firmware(char *key);
extern int set_host_info(struct in_addr ip, char *nick_name, char *name, unsigned char os_type, uint16_t vender);
extern unsigned char *dump_url_host(int *nr);
extern struct http_host_log *dump_http_log(unsigned char *mac, int *nr);
extern int set_host_url_switch(int act);
extern int mtd_set_val(char *val);
extern int mtd_get_val(char *name, char *val, int len);
extern int set_wps_led(int action);
extern int register_http_rsp(int type, struct url_rd *rd);
extern int unregister_http_rsp(int type);
#endif

#ifndef _DNS_MASQUE_
#define _DNS_MASQUE_

#define DHCP_CONFIG "dhcp"
#define DNSMASQ_CONFIG_FILE "/tmp/dnsmasq.conf"
#define DNSMASQ_CMD "dnsmasq -C /tmp/dnsmasq.conf -8 /tmp/dnsmasq_log -q &"

struct dhcp_conf {
	int enable;
	struct in_addr ip;
	struct in_addr mask;
	struct in_addr start;
	struct in_addr end;
};

#define DNSMASQ_MOD_INIT DEFINE_MSG(MODULE_DNSMASQ, 0)

enum {
	DNSMASQ_DHCP_SET = DEFINE_MSG(MODULE_DNSMASQ, 1),
	DNSMASQ_DHCP_SHOW,
};

extern int dnsmasq_call(MSG_ID msgid, void *data, int len, void *rbuf, int rlen);
#endif
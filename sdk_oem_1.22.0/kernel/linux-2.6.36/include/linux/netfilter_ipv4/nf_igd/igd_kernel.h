#ifndef __NETFILTER_IGD_KERNEL__H__
#define __NETFILTER_IGD_KERNEL__H__

#define NETDEV_STAT_BIT	0
#define NETDEV_WIFI_BIT	1
#define NETDEV_VIR_BIT	2
#define NETDEV_APCLI_BIT 3


/*  skbuff.h */

#define SKB_QOS_PASS  		0
#define SKB_QOS_ENQUEUED 	1
#define SKB_HTTP		2
#define SKB_LOCAL		3
#define SKB_ERR_ROUTE		4
#define SKB_TB			5
#define SKB_SKIP_QOS		6
#define SKB_PASS		7
#define SKB_TCP_MID		8
#define SKB_HTTP_REBUILD	9

struct skb_priv {
	__be32 ip;
	unsigned long flags;
	void *http;
	uint32_t next_seq;
	uint16_t dlen;
	uint16_t in_vport;
	char __pad[42]; /* reserved for feture use*/
};

/* nf_conntrack.h */
enum {
	CT_L7_DONE_BIT = 0,
	CT_L7_GATHER_BIT,
	CT_MATCH_URL_SAFE_BIT,
	CT_MATCH_FILTER_BIT,
	CT_MATCH_URL_BIT,
	CT_MATCH_MAX_BIT,//please add flages before this!
	CT_SKIP_FP_BIT = 12, //set this bit to skip fast path
	CT_SET_DST_BIT,
	CT_L7_DOING_BIT,
	CT_L7_INIT_BIT,
	CT_HTTP_BIT,
	CT_NF_NET_FP_BIT,
	CT_DROP_BIT,
	CT_FROM_WAN_BIT, /* conn come from wan */
	CT_URL_LOG_BIT,/*get url count*/
	CT_RST_BIT,
	CT_NET_PASS_BIT,
	CT_DST_PRIV_BIT,
	CT_NET_ACCEPT_BIT,
	CT_TCP_MID_BIT, /* in the middle of tcp */
	CT_LOCAL_BIT,
	CT_HTTP_REPLY_BIT,
	CT_DNS_CN_BIT,
	CT_MATCH_FLAGS_MASK = (0x1 << CT_MATCH_MAX_BIT) - 1,
};

enum {
	NF_MOD_DNS = 0,
	NF_MOD_LOCAL,
	NF_MOD_NET,
	NF_MOD_QOS,
	NF_MOD_RATE,
	NF_MOD_MX,
};

struct igd_filter {
        unsigned long flags;//which filter mod has match
        struct {
                uint32_t rematch;
                void *rule;
        } rules[NF_MOD_MX];
};

struct igd_url {
	unsigned long flags;
	short seq_off[2]; 
	u32 min_seq[2];
	u32 rep_last_seq;
	void *rule;
	void *data;
	uint16_t mid;
	uint16_t data_len;
};

struct igd_nf_conn {
	unsigned long flags;
	struct list_head list;
	struct list_head age_list;
	struct list_head app_list;
	void *app;
	void *http;
	void *host;
	__be32 ip;
	int tcp_wait; /*used by qos*/
	uint32_t syn[2];
	uint32_t fp_rematch;
	struct dst_entry *dst[2];
	void *tb;
	int def_appid; /* default appid */
#ifdef CONFIG_NF_L7
	uint32_t tcp_next_seq[2];
	void *l7;
	void *l7_http;
	void *l7_gr;
#endif
	/*new add entry must after this  */
	/*struct flow_stats stat;*/
	struct igd_filter filter;
	struct igd_url url;
	void *http_rebuild;
	unsigned long uptime;
	unsigned long last;
	unsigned char dns[32];
	uint32_t url_id;
	uint8_t queue; /* used by qos */
};

#ifndef NIPQUAD
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0],\
	((unsigned char *)&addr)[1],\
	((unsigned char *)&addr)[2],\
	((unsigned char *)&addr)[3]
#endif

extern void igd_kfree_skb(struct sk_buff *skb);
extern void igd_skb_clone(struct sk_buff *new, const struct sk_buff *old);
extern void igd_skb_init(struct sk_buff *skb);
extern void igd_wifi_work(unsigned char *mac, int action);
extern void wifi_pwd_nlkmsg(int type, uint8_t *mac, int stype);
extern int before_dev_hard_start_xmit(struct net_device *dev, struct sk_buff *skb);
extern int before_netif_receive_skb(struct net_device *dev, struct sk_buff *skb);
extern int before_dev_start_xmit(struct net_device *dev, struct sk_buff *skb);
extern int mirror_wifi;
extern int mirror_wifi_xmit(struct sk_buff *skb, int is_send);
extern void igd_ct_init(void *ct);
extern void igd_ct_destory(void *ct);

#endif


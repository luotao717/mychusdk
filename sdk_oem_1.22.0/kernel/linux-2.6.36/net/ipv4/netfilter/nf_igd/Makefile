obj-$(CONFIG_NF_IGD) += nf_igd.o nlk_msg_s.o igd_global.o nf_trie.o igd_grp.o igd_filter.o igd_url_match.o igd_url_rule.o igd_mirror.o igd_platform.o igd_dns_match.o
ifeq ($(CONFIG_RALINK_MT7628),y)
obj-$(CONFIG_NF_IGD) += igd_platform_7628.o
endif
ifeq ($(CONFIG_RALINK_MT7620),y)
obj-$(CONFIG_NF_IGD) += igd_platform_7620.o
endif
obj-$(CONFIG_NF_MATCH_HTTP) += nf_http.o


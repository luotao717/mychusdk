#include "nlk_ipc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void usage(void)
{
	printf("USAGE:\n");
	printf("\tget [bit]\n");
	printf("\tset [bit] [flag]\n");
	printf("\tsetplat [plat]\n");
	printf("\tgetplat\n");
	printf("\tsetkey\n");
	printf("\tgetkey\n");
	printf("\tsetpwd\n");
	printf("\tgetpwd\n");
	printf("\tsetmod\n");
	printf("\tgetmod\n");
	printf("\tsetname\n");
	printf("\tgetname\n");
	printf("DESCRIPTION:\n");
	printf("\tbit:0   reset\n");
	printf("\t    other nonsupport\n");
	printf("\tflag:0/1\n");
}

extern int get_sysflag(unsigned char bit);
extern int set_sysflag(unsigned char bit, unsigned char fg);
extern int get_platinfo(unsigned char *plat);
extern int set_platinfo(unsigned char plat);
extern int get_key(void);
extern int set_key(char *key);
extern int get_pwd(void);
extern int set_pwd(char *pwd);
extern int get_mod(void);
extern int set_mod(char *mod);
extern int get_devname(void);
extern int set_devname(char *devname);

int main(int argc, char *argv[])
{
	unsigned char plat = 0;
	int isset = 0, ret = 0;
	unsigned char bit = 0, fg = 0;

	if (!argv[1]) {
		usage();
		goto err;
	} else if (!strcmp(argv[1], "set")) {
		if (!argv[2] || !argv[3]) {
			printf("input err\n");
			goto err;
		}
		bit = (unsigned char)atoi(argv[2]);
		fg = (unsigned char)atoi(argv[3]);
		isset = 1;
	} else if (!strcmp(argv[1], "get")) {
		if (!argv[2]) {
			printf("input err\n");
			goto err;
		}
		bit = (unsigned char)atoi(argv[2]);
		isset = 0;
	} else if (!strcmp(argv[1], "setplat")) {
		if (!argv[2]) {
			printf("input err\n");
			goto err;
		}
		plat = (unsigned char)atoi(argv[2]);
		ret = set_platinfo(plat);
		if (ret) {
			printf("set plat error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "getplat")) {
		ret = get_platinfo(&plat);
		if (ret) {
			printf("get plat error\n");
			goto err;
		}
		printf("plat is %d\n", plat);
		return ret;
	} else if (!strcmp(argv[1], "setkey")) {
		if (!argv[2]) {
			printf("input err\n");
			goto err;
		}
		ret = set_key(argv[2]);
		if (ret) {
			printf("set key error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "getkey")) {
		ret = get_key();
		if (ret) {
			printf("get key error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "setpwd")) {
		if (!argv[2]) {
			printf("input err\n");
			goto err;
		}
		ret = set_pwd(argv[2]);
		if (ret) {
			printf("set pwd error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "getpwd")) {
		ret = get_pwd();
		if (ret) {
			printf("get pwd error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "setmod")) {
		if (!argv[2]) {
			printf("input err\n");
			goto err;
		}
		ret = set_mod(argv[2]);
		if (ret) {
			printf("set mod error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "getmod")) {
		ret = get_mod();
		if (ret) {
			printf("get mod error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "setname")) {
		if (!argv[2]) {
			printf("input err\n");
			goto err;
		}
		ret = set_devname(argv[2]);
		if (ret) {
			printf("set devname error\n");
			goto err;
		}
		return ret;
	} else if (!strcmp(argv[1], "getname")) {
		ret = get_devname();
		if (ret) {
			printf("get devname error\n");
			goto err;
		}
		return ret;
	}
	if ((bit >= SYSFLAG_MAX) || 
		((fg != 0) && (fg != 1))) {
		printf("input err\n");
		goto err;
	}

	if (isset) {
		ret = set_sysflag(bit, fg);
		if (ret < 0) {
			printf("set bit(%d) fg(%d) fail\n", bit, fg);
			return -1;
		}
	} else {
		ret = get_sysflag(bit);
		if (ret < 0) {
			printf("get bit(%d) fail\n", bit);
			return -1;
		}
		printf("sysflag:bit:%d, flag:%d\n", bit, ret);
	}
	return 0;
err:
	return -1;
}

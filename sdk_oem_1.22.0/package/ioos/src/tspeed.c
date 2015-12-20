#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdarg.h> 
#include <errno.h> 
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/timeb.h>
#include <netdb.h>
#include "common.h"
#include "ioos_uci.h"
#include "nlk_ipc.h"

#define TSPEED_PRINTF  printf
#define TESTSPEED_FILE  "/tmp/testspeed"

static char *httphead="GET %s HTTP/1.0\r\n"
"User-Agent:  wget/1.12 (linux-gnu)\r\n"
"Accept: */*\r\n"
"Connection: Keep-Alive\r\n"
"Host: %s\r\n\r\n";

static unsigned char wlock = 0;
static unsigned char ping_nr = 0, down_nr = 0;

static int tcp_connect(const char *host, const char *serv)
{
	int sockfd = -1;
	struct addrinfo hints, *iplist = NULL, *ip = NULL;

	bzero(&hints,sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(host, serv, &hints, &iplist) != 0 ){
		return -1;
	}

	for (ip = iplist; ip != NULL; ip = ip->ai_next) {
		sockfd = socket(ip->ai_family, ip->ai_socktype, ip->ai_protocol);
		if (sockfd < 0)
			continue;
		if (connect(sockfd, ip->ai_addr, ip->ai_addrlen) == 0)
			break;
		close(sockfd);
	}
	freeaddrinfo(iplist);
	return sockfd;
}

static int gethttp(char *url, char *newurl)
{
	char host[256] = {0,}, hurl[1024] = {0,}, buf[4096] = {0,};
	char *port = NULL, *p = NULL, *pend = NULL;
	int len = 0, n = 0, sock_fd = 0, revn = 0;

	p = url + strlen("http://");
	pend = strchr(p, '/');
	if (pend == NULL) {
		return -1;
	}
	len = pend - p;
	snprintf(host, 256, "%.*s", len, p);
	p = pend;
	pend = strchr(p, '&');
	if (pend == NULL) {
		len = strlen(p);
	}else{
		len = pend - p;
	}
	snprintf(hurl,1024,"%.*s",len,p);
	pend = strchr(host,':');
	if (pend != NULL) {
		*pend++ = '\0';
		port = pend;
	} else {
		port = "80";
	}
	sock_fd = tcp_connect(host,port);
	if (sock_fd < 0) {
		TSPEED_PRINTF("tcp connect fail\n");
		return -1;
	}
	sprintf(buf, httphead, hurl, host);
	n = write(sock_fd, buf, strlen(buf));
	while (1) {
		n = read(sock_fd, &buf[revn], 1024 - revn);
		if (n <= 0) {
			TSPEED_PRINTF("recv error\n");
			close(sock_fd);
			return -1;
		}
		revn += n;
		if (revn > 4 && (pend =strstr(buf,"\r\n\r\n"))!= NULL) {
			if (buf[9] == '3') {
				char *clenp = strstr(buf,"Location:");
				char *endp = strchr(clenp,'\r');
				sprintf(newurl, "%.*s", endp - clenp - strlen("Location: "), clenp+strlen("Location: "));
				close(sock_fd);
				return 1;
			} else {
				break;
			}
		}
		break;
	}

	while (1) {
		n = read(sock_fd, buf, 4096);
		if (n <= 0) {
			close(sock_fd);
			return 0;
		}
		revn += n;
	}
}

static void *get_url(void *url)
{
	char newurl[2048], oldurl[2048];

	strcpy(oldurl, url);
	free(url);

	down_nr++;
	while (1) {
		if (gethttp(oldurl, newurl) == 1) {
			TSPEED_PRINTF("url:%s\nnewurl:%s\n", oldurl, newurl);
			strcpy(oldurl, newurl);
		} else {
			break;
		}
	}
	down_nr--;
	return NULL;
}

static void thread_fun(char *url, void *(*fun)(void *))
{
	pthread_t tid;
	char *turl = NULL;
	int ret = -1;

	turl = strdup(url);
	if (turl == NULL) {
		TSPEED_PRINTF("strdup fail\n");
		return;
	}
	ret = pthread_create(&tid, NULL, fun, turl);
	TSPEED_PRINTF("pthread:ret=%d, tid:%X, url:%s\n", ret, (int)tid, turl);
}

static int tspeed_start(char *file, char *flag, void *(*fun) (void *))
{
	FILE *fp = NULL;
	char line[2048], *p = NULL;

	fp = fopen(file, "r");
	if (fp == NULL) {
		return -1;
	}
	memset(line, 0, sizeof(line));
	while (fgets(line, sizeof(line) - 1, fp)) {
		if (memcmp(line, flag, strlen(flag)))
			continue;
		p = line + strlen(line) - 1;
		while (p > line) {
			if ((*p == '\r') || (*p == '\n') ||
					(*p == ' ') || (*p == '\0')) {
				*p = '\0';
				p--;
			} else {
				break;
			}
		}
		if (strlen(line) > 0) {
			thread_fun(line + strlen(flag), fun);
		}
		memset(line, 0, sizeof(line));
	}
	fclose(fp);
	return 0;
}

static int tspeed_state(char *state)
{
	FILE *fp = NULL;

	while(wlock) {
		usleep(100000);
	}
	wlock = 1;
	fp = fopen(TESTSPEED_FILE, "a+");
	if (fp) {
		fprintf(fp, "%s\n", state);
		fclose(fp);
	}
	wlock = 0;
	return 0;
}

#define PING_NUM  5
static unsigned int ping_time = 0, ping_delay = 0; 
static void *ping_url(void *url)
{
	int sockfd = -1, i = 0, j = 0;
	struct addrinfo hints, *iplist = NULL, *ip = NULL;
	struct timeb tb, newtb;
	unsigned int tm[PING_NUM] = {0,};

	ping_nr++;
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if (getaddrinfo(url, "80", &hints, &iplist) != 0) {
		free(url);
		ping_nr--;
		return NULL;
	} else {
		TSPEED_PRINTF("getaddrinfo success\n");
	}

	for (i = 0; i < PING_NUM; i++) {
		ftime(&tb);
		newtb = tb;
		if (iplist) {
			ip = iplist;
		} else {
			break;
		}
		sockfd = socket(ip->ai_family, ip->ai_socktype, ip->ai_protocol);
		if (sockfd < 0)
			continue;
		if (connect(sockfd, ip->ai_addr, ip->ai_addrlen) == 0) {
			ftime(&newtb);
		} else {
			TSPEED_PRINTF("connect fail\n");
		}
		close(sockfd);
		if (i - j > 1)
			break;
		tm[i] = (newtb.time - tb.time)*1000 + newtb.millitm - tb.millitm;
		if (tm[i]) {
			ping_delay += tm[i];
			ping_time++;
			j++;
		}
		usleep(500000);
	}
	freeaddrinfo(iplist);
	free(url);
	ping_nr--;
	return NULL;
}

static void kill_pre(void)
{
	FILE *fp = NULL;
	char line[2048];
	pid_t pid = 0;

	fp = fopen(TESTSPEED_FILE, "r");
	if (fp == NULL)
		return;
	memset(line, 0, sizeof(line));
	while (fgets(line, sizeof(line) - 1, fp)) {
		if (!memcmp(line, "START", 5)) {
			pid = atoi(line + 6);
			break;
		}
	}
	if (pid > 0) {
		kill(pid, SIGTERM);
	}
	remove(TESTSPEED_FILE);
}

static int downspeed = 0;
static void speed_timer(int sig)
{
	char buf[512] = {0,};

	snprintf(buf, sizeof(buf) - 1, "DELAY=%u,%d", \
		(ping_time == 0) ? 0 : ping_delay/ping_time, ping_time);
	tspeed_state(buf);

	sprintf(buf, "DOWN_SPEED=%d", downspeed);
	tspeed_state(buf);

	sprintf(buf, "END=%d", getpid());
	tspeed_state(buf);
	system("/usr/bin/qos-start.sh restart");
	exit(0);
}

int main(int argc, char *argv[])
{
	struct if_statistics statis;
	int tmp = 0, i = 0, time = 0;
	char buf[512] = {0,};

	dm_daemon();
	kill_pre();
	wlock = 0;
	sprintf(buf, "START=%d", getpid());
	tspeed_state(buf);

	system("/usr/bin/qos-start.sh stop");
	tspeed_start(argv[1], "PING:", ping_url);
	usleep(100000);

	signal(SIGALRM, speed_timer);
	alarm(40);

	time = 0;
	while ((ping_nr > 0) && (time < 5)) {
		time++;
		sleep(1);
	}

	tspeed_start(argv[1], "DOWN:", get_url);
	sleep(1);
	for (i = 0; (i < 30) && down_nr; i++) {
		if (get_if_statistics(1, &statis) != 0) {
			tmp = 0;
		} else {
			tmp = statis.in.all.speed/1024;
		}
		TSPEED_PRINTF("speed:%d, down_nr:%d\n", tmp, down_nr);
		if (tmp > downspeed) {
			downspeed = tmp;
		}
		sleep(1);
	}
	while (1) {
		sleep(1);
	}
	return 0;
}

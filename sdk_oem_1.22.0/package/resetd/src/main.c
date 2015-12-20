/*****************************************************************************
* $File:   watchdog.c
*
* $Author: Hua Shao
* $Date:   Feb, 2014
*
* The dog needs feeding.......
*
*****************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "wd_com.h"

void wd_dbg(const char *fmt, ...)
{
	FILE *fp = NULL;
	va_list ap;

	fp = fopen("/dev/console", "w");
	if (fp) {
		va_start(ap, fmt);
		vfprintf(fp, fmt, ap);
		va_end(ap);
		fclose(fp);
	}
}

static unsigned char _running = 1;
static void sigterm_hdl(int arg)
{
    _running = 0;
}

static void sigsur_hdl(int arg)
{
	if (arg == SIGUSR1)
		wps_run();
}

int main(int argc, char *const argv[])
{
	pid_t pid = 0;
	int flag = argv[1] ? atoi(argv[1]) : 1;

	pid = fork();
	if (pid < 0) {
		WD_DBG("fork fail!,%m\n");
		return -1;
	} else if (pid > 0) {
		exit(0);
	}

	sleep(10);
	WD_DBG("reset start\n");

	signal(SIGTERM, sigterm_hdl);
	signal(SIGUSR1, sigsur_hdl);
	signal(SIGUSR2, sigsur_hdl);

	wps_init();
	resetd_init(flag ? flag : 1);

	while(_running) {
		resetd_loop();
		usleep(100000);
	}

	WD_DBG("reset quit\n");
	return 0;
}

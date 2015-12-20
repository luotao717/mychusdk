#include <stdio.h>
#include <igd_lib.h>
#include <igd_system.h>

#define CHECK_FILE  "/proc/intreg"

#define RALINK_GPIO_ENABLE_INTP		0x08
#define RALINK_GPIO_SET_DIR_IN		0x11
#define RALINK_GPIO_REG_IRQ		0x0A
typedef struct {
	unsigned int irq;		//request irq pin number
	pid_t pid;			//process id to notify
} ralink_gpio_reg_info;

static void signal_handler(int signum)
{
	/*  do nothing*/
	return ;
}

int main(int argc, char *argv[])
{
	char buf[64] = {0};
	int fd = 0, rbyts = 0, time = 0;
	ralink_gpio_reg_info info;

	system("mknod /dev/gpio c 252 0");
	fd = open("/dev/gpio", O_RDONLY);
	if (fd < 0) {
		perror("/dev/gpio");
		return -1;
	}
	/* set gpio direction to input */
	ioctl(fd, RALINK_GPIO_SET_DIR_IN, 1<<1);
	/* enable gpio interrupt */
	ioctl(fd, RALINK_GPIO_ENABLE_INTP);
	/* register my information */
	info.pid = getpid();
	info.irq = 1;

	ioctl(fd, RALINK_GPIO_REG_IRQ, &info);
	close(fd);
	fd = 0;
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	while (1) {
		usleep(100000);
		if (fd <= 0)
			fd = open(CHECK_FILE, O_RDONLY);
		if (fd <= 0)
			continue;
		memset(buf, 0, sizeof(buf));
		//press:0
		lseek(fd, 0, SEEK_SET);
		rbyts = read(fd, buf, sizeof(buf));
		if (rbyts < 0) {
			close(fd);
			fd = -1;
			continue;
		} else if (rbyts < 7) {
			continue;
		}
		if (atoi(&buf[6]) == 1) {
			console_printf("press reset\n");
			time++;
		} else {
			time = 0;
		}
		if (time > 30) {
			system("killall mu");
			system("killall pppd");
			usleep(2000000);
			set_sysflag(SYSFLAG_RESET, 1);
			if (!access("/etc/init.d/ali_cloud.init", F_OK))
				set_sysflag(SYSFLAG_ALIRESET, 1);
			system("mtd -r erase \"rootfs_data\"");
			break;
		}
	}
	return 0;
}

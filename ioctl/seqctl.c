#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../seq_ioctl.h"

static int fd;

void prepare()
{
	fd = open(SEQ_PATH, O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(-1);
	}
}

void cleanup()
{
	close(fd);
}

int get_int(int cmd)
{
	int res;
	if (ioctl(fd, cmd, &res) == -1) {
		perror("ioctl");
		cleanup();
		exit(-1);
	}
	return res;
}

char get_char(int cmd)
{
	char res;
	if (ioctl(fd, cmd, &res) == -1) {
		perror("ioctl");
		cleanup();
		exit(-1);
	}
	return res;
}

void set_char(int cmd, char c)
{
	if (ioctl(fd, cmd, &c) == -1) {
		perror("ioctl");
		cleanup();
		exit(-1);
	}
}

void show_usage(char *name)
{
	printf("Usage: %s\tget (begin|end|step|delimiter)\n"
	       "\t\tset delimiter <delimiter>\n",
	       name);
}

int main(int argc, char **argv)
{
	prepare();

	if (argc == 3 && !strcmp("get", argv[1])) {
		if (!strcmp("begin", argv[2])) {
			printf("%d\n", get_int(SEQ_IOCTL_GET_BEGIN));
			cleanup();
			return 0;
		}
		if (!strcmp("end", argv[2])) {
			printf("%d\n", get_int(SEQ_IOCTL_GET_END));
			cleanup();
			return 0;
		}
		if (!strcmp("step", argv[2])) {
			printf("%d\n", get_int(SEQ_IOCTL_GET_STEP));
			cleanup();
			return 0;
		}
		if (!strcmp("delimiter", argv[2])) {
			printf("%c\n", get_char(SEQ_IOCTL_GET_DELIMITER));
			cleanup();
			return 0;
		}
	}

	if (argc == 4 && !strcmp("set", argv[1])) {
		if (!strcmp("delimiter", argv[2])) {
			set_char(SEQ_IOCTL_SET_DELIMITER, argv[3][0]);
			cleanup();
			return 0;
		}
	}

	cleanup();
	show_usage(argv[0]);
	return -1;
}


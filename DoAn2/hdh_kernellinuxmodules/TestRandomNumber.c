#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

int main(int argc, char** argv)
{
	int fd;
	int result,rt;

	if (argc == 1)
		fd = open("/dev/RandomNumber", 0);
	else
		fd = open(argv[1], 0);    
	rt = read(fd, &result, sizeof(result));
	printf("Random number: %d\n", rt);
	close(fd);

	return 0;
}
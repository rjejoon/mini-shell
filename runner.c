#include <stdio.h>		/* defines printf() */
#include <stdlib.h>		/* to define exit() */
#include <unistd.h>		/* for sysconf() */
#include <stdint.h>		/* used for casting clock_t to uintmax_t for printf */
#include <sys/times.h>		/* needed for the times() system call */
#include <sys/types.h>
#include <sys/stat.h>		/* used for open() */
#include <fcntl.h>		/* used for open() */

int main()
{
    long long i, j=0;	/* use these in a loop to kill time */
	int fd;
	char buf[2048];

	printf("doing some cpu wasting stuff\n");
	for (i=0; i<1500000000; i++)
		j += i;

	printf("doing some kernel wasting stuff\n");
	/* do some stuff to waste system time */
	if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
		perror("open");
		exit(1);
	}
	for (i=0; i < 1000000; i++)
		read(fd, buf, 2048);
	close(fd);
}

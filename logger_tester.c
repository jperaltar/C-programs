#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

int
main(int argc, char* argv[])
{
	int fd, nr;
	char buf[1024];
	
	for(;;){
		fd = open("/tmp/logger", O_WRONLY);
		if (fd < 0){
			err(1, "open");
		}
		nr = read(0, buf, sizeof (buf) -1);
		buf[nr] = 0;
		write(fd, buf, nr);
		if(strcmp(buf, "bye\n") == 0){
			close(fd);
			break;
		}
	}
	exit(0);
}
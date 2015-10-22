#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

static int
pipefrom(char *argv[])
{
	int fd[2];

	pipe(fd);
	switch(fork()){
	case -1:
		return -1;
	case 0:
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		execv("/bin/gunzip", argv);
		err(1, "execv");
	default:
		close(fd[1]);
		return fd[0];
	}
}


int
main (int argc, char *argv[])
{
	int nr, fd, fdout;
	int stdout = 1;
	int sizeofbuf = 1024;
	char buf[sizeofbuf];
	
	argv++;
	argc--;
	
	fd = pipefrom(argv);
	if(argc == 1){
		fdout = creat(argv[0], 0644);
		if(fdout < 0){
			err(1,"creat file");
		}
	}
	for (;;){
		nr = read (fd, buf, sizeofbuf);
		if (nr < 0){
			err(1, "failed reading");
		}else if (nr == 0){
			break;
		}else{
			if (write(stdout, buf, nr) < 0){
				err(1,"std writting");
			}
			if(argc == 1){
				if (write(fdout, buf, nr) < 0){
					err(1,"file writting");
				}
			}
		}
	}
	close(fdout);
	exit(0);
}
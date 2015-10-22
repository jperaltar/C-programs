#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>



static void
printdate(char *buf)
{
	time_t t;
	char *ct;
	int size_time;

	t = time(&t);
	ct = ctime(&t);
	size_time = strlen(ct);
	ct[size_time-1] = 0;
	printf("%s: %s", ct, buf);
	fflush(stdin);
}

static void
tout(int no)
{
	char *msg = "time out, no events\n";

	printdate(msg);
	signal(SIGALRM, tout);
	siginterrupt(SIGALRM, 0);
	alarm(5);
}

static void
readfifo(int fd)
{
	char buf[1024];
	int nr;

	for(;;){
		signal(SIGALRM, tout);
		siginterrupt(SIGALRM, 0);
		alarm(5);
		nr = read(fd, buf, sizeof buf - 1);
		if(nr < 0){
			err(1, "read");
		}else if(nr == 0){
			break;
		}
		buf[nr] = 0;
		alarm(0);
		printdate(buf);
		if(strcmp(buf, "bye\n") == 0){
			close(fd);
			exit(0);
		}
	}
}

int
main(int argc, char* argv[])
{
	int fd;
	
	if(!access("/tmp/logger", F_OK)){
		if(unlink("/tmp/logger") < 0){
			err(1,"unlink");
		}
	}
	if(mkfifo("/tmp/logger", 0664) < 0)
		err(1, "mkfifo");
	for(;;){
		printf("waiting for clients\n");
		fd = open("/tmp/logger", O_RDONLY);
		if(fd < 0) {
			err(1, "open");
		}
		printf("ready to read events\n");
		readfifo(fd);
		alarm(0);
		close(fd);
	}
	exit(0);
}

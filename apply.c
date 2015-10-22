#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

/*This program applys a command entered from the standard input
  to all the archives ".txt" in the current directory and redirects
  the standard output to a file "apply.output"*/


static int
execute(char *command, char *argv[], int argc, char *dir_name)
{
	int pid, sts, fd;

	pid = fork();
	switch (pid){
	case -1:
		return -1;
	case 0:
		fd = open(dir_name, O_RDONLY);
		dup2(fd, 0);
		close(fd);
		execv(command, argv);
		err(1, "exec %s failed", command);
	default:
		while(wait(&sts) != pid){
			;
		}
		return sts;
	}
}

static char*
full_path(char *location, char *command)
{
	char *path = malloc (265);

	snprintf(path, 265, "%s/%s", location, command);
	return path;
}

static char*
find_command(char *command)
{
	int N_Paths = 3;
	int i;
	char *locations[] = {"/bin", "/usr/bin", "/usr/local/bin"};
	char *path;
	
	for(i=0; i < N_Paths; i++){
		path = full_path(locations[i], command);
		if(access(path, X_OK) == 0){
			break;
		}
	}
	return path;
}

static void
execute_in_txt_files (DIR *d, char *command, char* argv[], int argc)
{
	struct dirent *de;
	struct stat st;
	char *format = ".txt";
	int fdout;
	
	fdout = creat("apply.output", O_TRUNC);
	fchmod(fdout, 0777);
	dup2(fdout, 1);
	close(fdout);
	while((de = readdir(d)) != NULL){
		if(stat(de->d_name, &st) < 0){
			err(1, "%s", de->d_name);
		}
		if((st.st_mode & S_IFMT) == S_IFREG){
			if(strstr(de->d_name, format) != NULL){
				execute(command, argv, argc, de->d_name);
			}
		}
	}
	free(command);
}

int
main(int argc, char* argv[])
{
	DIR *d;
	char *actual_dir = ".";

	argv++;
	argc--;
	
	d = opendir(actual_dir);
	if(d == NULL){
		err(1, "opening actual dir");
	}
	execute_in_txt_files(d, find_command(argv[0]), argv, argc);
	exit(0);
}
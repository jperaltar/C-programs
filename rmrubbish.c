#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

static char*
path_file_to_remove(char *dir)
{
	char actdir[1024];
	char *file_to_remove;

	getcwd(actdir, sizeof(actdir));
	file_to_remove = strcat(actdir, "/");
	file_to_remove = strcat(file_to_remove, dir);
	return file_to_remove;
}

static void
remove_file (char *dir)
{
	char *file_to_remove;

	file_to_remove = path_file_to_remove(dir);
	if (unlink(file_to_remove) < 0){
		fprintf(stderr,"error removing %s\n", file_to_remove);
		exit(1);
	}
}

static int
go_over_dir_stream (DIR *d, char* format)
{
	DIR *subd;
	struct dirent *de;
	struct stat st;
	char *dir;
	char *actual = ".";
	char *prev = "..";
	int removed_files = 0;

	while((de = readdir(d)) != NULL) {
		dir = de->d_name;
		if (stat(de->d_name, &st) < 0) {
			err(1, "%s", dir);
		}
		if (st.st_mode & S_IFDIR){
			if (strcmp(dir, actual) && strcmp(dir, prev)){
				subd = opendir(dir);
				if (chdir(dir) < 0){
					err(1,"chdir %s", dir);
				}
				removed_files = go_over_dir_stream(subd, format);
				if (chdir(prev) < 0){
					err(1,"chdir %s", prev);
				}
				closedir(subd);
			}
		}else {
			if (strstr(dir, format) != NULL){
				remove_file (dir);
				removed_files ++;
			}
		}
	}
	return removed_files;
}

static void
dirs_parallel_checking (int dirs_num, char* dirs[])
{
	int i;
	int pid, sts;
	int child_pid;
	int removed_files;
	int successful_processes = 0;
	int failed_processes = 0;
	char *prev = "..";
	char *format = ".rubbish";
	DIR *d;
	
	for(i = 1; i < dirs_num; i++){
		pid = fork();
		switch(pid){
		case -1:
			err(1, "fork");
		case 0:
			child_pid = getpid();
			d = opendir(dirs[i]);
			if(d == NULL){
				err(1, "%s", dirs[i]);
			}
			if(chdir(dirs[i]) < 0){
				err(1, "chdir %s", dirs[i]);
			}
			removed_files = go_over_dir_stream(d, format);
			if(removed_files != 0){
				fprintf(stderr,"%d: %s ok\n", child_pid, dirs[i]);
				exit(0);
			}else {
				fprintf(stderr,"%d: no files to remove in dir %s\n", child_pid, dirs[i]);
				exit(1);
			}
			if(chdir(prev) < 0){
				err(1, "chdir %s", prev);
			}
			closedir(d);
		}
	}
	while (wait(&sts) != pid){
		;
	}
	if (sts == 0){
		successful_processes ++;
	}
	failed_processes = (dirs_num-1) - successful_processes;
	if (failed_processes == 0){
		/*Esperamos 10ms para que el resultado se escriba el ultimo*/
		usleep(10000);
		printf("all processes were successful\n");
	}else {
		/*Esperamos 10ms para que el resultado se escriba el ultimo*/
		usleep(10000);
		printf("%d processes failed\n", failed_processes);
	}
}

int
main(int argc, char* argv[])
{	
	if (argc > 1){
		dirs_parallel_checking(argc, argv);
	}else {
		err(1,"Not enough arguments");
	}
	usleep(20);
	exit(0);
}
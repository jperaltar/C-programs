#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <dirent.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

static void
go_over_dir_stream (DIR *d, char* word)
{
	DIR *subd;
	struct stat st;
	struct dirent *de;
	int file = 0;
	int nr = 0;
	int Sizeofword = strlen(word);
	char *actual = ".";
	char *prev = "..";
	char *dir;
	char buf[Sizeofword];
	char actdir[1024];
	
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
				go_over_dir_stream(subd, word);
				if (chdir(prev) < 0){
					err(1,"chdir %s", prev);
				}
				closedir(subd);
			}
		}else {
			file = open (dir,O_RDONLY);
			nr = read (file, buf, Sizeofword);
			if (nr < 0){
				err(1, "read");
			}
			if (!strncmp(buf, word, Sizeofword)){
				getcwd(actdir, sizeof(actdir));
				printf("%s", actdir);
				printf("/%s\n\n", dir);

			}
			close(file);
		}
	}
}

static void
dirs_checking_list (int dirs_num, char* dirs[], char* word)
{
	int i = 2;
	char *prev = "..";
	DIR *d;
	
	while (i < dirs_num){
		d = opendir(dirs[i]);
		if(d == NULL) {
			err(1, ".");
		}
		if (chdir(dirs[i]) < 0){
			err(1,"chdir %s", dirs[i]);
		}
		go_over_dir_stream(d, word);
		if (chdir(prev) < 0){
			err(1,"chdir %s", prev);
		}
		closedir(d);
		i++;
	}
}

int
main(int argc, char* argv[])
{
	DIR *d;
	
	printf("\n");
	if (argc > 2){
		dirs_checking_list(argc, argv, argv[1]);
	}else if (argc == 2){
		d = opendir(".");
		go_over_dir_stream(d, argv[1]);
		closedir(d);
	}else {
		err(1,"Not enough arguments");
	}
	exit(0);
}
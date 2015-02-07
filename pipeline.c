#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>


/*Concatenates shell commands and redirects
  std input and output*/

enum{
	N = 10,
};

struct commands{
	int stdin;
	int stdout;
	char **commands;
	int pipes[N][2];
};

typedef struct commands commands;

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
runprocess(commands *cml, int com_num, int num_commands)
{
	char *command;
	char *argv[2];

	switch(fork()){
	case -1:
		err(1,"fork");
		exit(1);
	case 0:
		if(com_num == 0){
			if(dup2(cml->stdin, 0)<0){
				err(1,"dup2-1");
			}
			if(close(cml->stdin)<0){
				err(1,"close");
				exit(1);
			}
			if(num_commands > 1){
				if(dup2(cml->pipes[com_num][1], 1)<0){
					err(1,"dup2-2");
				}
			}else{
				if(dup2(cml->stdout, 1)<0){
					err(1,"dup2-4");
				}
				if(close(cml->stdout)<0){
					err(1,"close");
					exit(1);
				}
			}
		}else if(com_num == num_commands-1){
			if(dup2(cml->pipes[com_num-1][0], 0)<0){
				err(1,"dup2-3");
			}
			if(dup2(cml->stdout, 1)<0){
				err(1,"dup2-4");
			}
			if(close(cml->stdout)<0){
				err(1,"close");
				exit(1);
			}
		}else{
			if(dup2(cml->pipes[com_num-1][0], 0)<0){
				err(1,"dup2-5");
			}
			if(dup2(cml->pipes[com_num][1], 1)<0){
				err(1,"dup2-6");
			}
		}
		command = find_command(cml->commands[com_num]);
		argv[0] = cml->commands[com_num];
		argv[1] = NULL;
		execv(command, argv);
		err(1, "exec %s failed", command);
	default:
		if(com_num == 0){
			if(close(cml->pipes[com_num][1])<0){
				err(1,"Parent close");
				exit(1);
			}
		}else if(com_num == num_commands-1){
			if(close(cml->pipes[com_num-1][0])<0){
				err(1,"Parent close");
			}
		}else{
			if(close(cml->pipes[com_num-1][0])<0){
				err(1,"Parent close");
				exit(1);
			}
			if(close(cml->pipes[com_num][1])<0){
				err(1,"Parent close");
				exit(1);
			}
		}
	}
}

static void
pipelining(commands *cml, int num_commands)
{
	int i, fd[2], sts;

	for (i=0; i < num_commands-1; i++){
		pipe(fd);
		cml->pipes[i][0] = fd[0];
		cml->pipes[i][1] = fd[1];
	}

	for (i=0; i < num_commands; i++){
		runprocess(cml, i, num_commands);
	}

	for (i = 0; i < num_commands; i++){
		wait(&sts);
	}
}

int
main (int argc, char *argv[])
{
	int i, fdout, fdin;
	char outflag[] = "-o";
	char inflag[] = "-i";
	char *outfile, *infile;
	commands *com_line = (commands*) malloc (sizeof(commands));
	argc--;
	argv++;
	
	if (argc < 6 || argc > N){
		err(1, "No apropriate number of commands");
		exit(1);
	}else{
		for (i=0; i < 5; i++){
			if (!strcmp(argv[i], outflag)){
				i++;
				outfile = argv[i];
				fdout = creat(outfile, O_WRONLY);
				com_line->stdout = fdout;
			}else if (!strcmp(argv[i], inflag)){
				i++;
				infile = argv[i];
				fdin = open(infile, O_RDONLY);
				com_line->stdin = fdin;
			}
		}
	
		argc=argc-4;
		argv=argv+4;
		com_line->commands = argv;
	
		pipelining(com_line, argc);
	}
	exit(0);
}
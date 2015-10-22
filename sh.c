#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>

enum{
	N = 10,
};

struct commands{
	int stdin;
	int stdout;
	char **piped;
	int pipes[N][2];
};

typedef struct commands commands;

static int
argsdivider(char *string, char **args, char *dividers)
{
	int i;
	int counter = 0;
	char *tok, *saved;

	for (i=0,tok = strtok_r(string, dividers, &saved);
		tok; tok = strtok_r(NULL, dividers, &saved),i++){
		args[i] = tok;
		counter++;
	}
	return counter;
}

static void
reunify_value(int varsnum, char **vars)
{
	int i;
	char *aux;

	aux = vars[0];
	for(i = 1; i < varsnum; i++){
		vars[0] = (char *) malloc (strlen(aux)+strlen(vars[i])+1);
		strcat(vars[0], aux);
		strcat(vars[0], vars[i]);
		aux = vars[0];
	}
}

static void
change_value(int varsnum, char **vars, int skipfirst)
{
	int i;
	char *namevar;

	for(i = 0; i < varsnum; i++){
		if(skipfirst){
			skipfirst = 0;
			continue;
		}
		namevar = vars[i];
		vars[i] = getenv(vars[i]);
		if(vars[i] == NULL){
			errx(1, "error: variable %s does not exist", namevar);
		}
	}
}

static void
identify_vars(int argsnum, char **args)
{
	int i, varsnum;
	int skipfirst;
	char *argv[20];
	char *divider = "$";

	for(i = 0; i < argsnum; i++){
		if(args[i][0] != divider[0]){
			skipfirst = 1;
		}else{
			skipfirst = 0;
		}
		varsnum = argsdivider(args[i], argv, "$");
		change_value(varsnum, argv, skipfirst);
		reunify_value(varsnum, argv);
		args[i] = argv[0];
	}
}

static void
assign_vars(char **args)
{
	setenv(args[0], args[1], 1);
}

static void
change_dir(int argsnum, char **args)
{
	char *home;

	if(argsnum == 1){
		home = getenv("HOME");
		chdir(home);
	}else{
		if(chdir(args[1]) < 0){
			warn("%s", args[1]);
		}
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
	int N_Paths = 100;
	int i;
	char *actualdir = ":.";
	char *paths;
	char *loc[N_Paths];
	char *aux;
	char *path;
	
	aux = getenv("PATH");
	paths = (char *) malloc (strlen(aux)+strlen(actualdir)+1);
	strcat(paths, aux);
	strcat(paths, actualdir);
	N_Paths = argsdivider(paths, loc, ":");

	for(i=0; i < N_Paths; i++){
		path = full_path(loc[i], command);
		if(access(path, X_OK) == 0){
			break;
		}
	}
	free(paths);
	return path;
}

static int
runpipeline(commands *cml, int procnum, int pipesnum)
{
	int argsnum, pid, childpid;
	char *command;
	char *argv[20];

	pid = fork();
	switch(pid){
	case -1:
		err(1,"fork");
		exit(1);
	case 0:
		if(pipesnum == 1){
			childpid = getpid();
			if(cml->stdin != 0){
				if(dup2(cml->stdin, 0)<0){
					err(1,"dup2-1");
				}
				if(close(cml->stdin)<0){
					err(1,"close");
				}
			}
			if(cml->stdout != 1){
				if(dup2(cml->stdout, 1)<0){
					err(1,"dup2-4");
				}
				if(close(cml->stdout)<0){
					err(1,"close");
				}
			}
		}else{
			if(procnum == 0){
				if(cml->stdin != 0){
					if(dup2(cml->stdin, 0)<0){
						err(1,"dup2-1");
					}
					if(close(cml->stdin)<0){
						err(1,"close");
					}
				}
				if(dup2(cml->pipes[procnum][1], 1)<0){
					err(1,"dup2-2");
				}
			}else if(procnum == pipesnum-1){
				childpid = getpid();
				if(dup2(cml->pipes[procnum-1][0], 0)<0){
					err(1,"dup2-3");
				}
				if(cml->stdout != 1){
					if(dup2(cml->stdout, 1)<0){
						err(1,"dup2-4");
					}
					if(close(cml->stdout)<0){
						err(1,"close");
					}
				}
			}else{
				if(dup2(cml->pipes[procnum-1][0], 0)<0){
					err(1,"dup2-5");
				}
				if(dup2(cml->pipes[procnum][1], 1)<0){
					err(1,"dup2-6");
				}
			}
		}
		argsnum = argsdivider(cml->piped[procnum], argv, " \n\r\t");
		//Cambio de variable a su valor
		identify_vars(argsnum, argv);
		command = find_command(argv[0]);
		argv[argsnum] = NULL;
		execv(command, argv);
		errx(1, "%s: Command not found", argv[0]);
	default:
		childpid = getpid();
		if(pipesnum > 1){
			if(procnum == 0){
				if(close(cml->pipes[procnum][1])<0){
					err(1,"Parent close 1");
				}
			}else if(procnum == pipesnum-1){
				if(close(cml->pipes[procnum-1][0])<0){
					err(1,"Parent close 2");
				}
			}else{
				if(close(cml->pipes[procnum-1][0])<0){
					err(1,"Parent close 3");
				}
				if(close(cml->pipes[procnum][1])<0){
					err(1,"Parent close 4");
				}
			}
		}
	}
	return childpid;
}

static void
pipelining(int nowait, commands *cml, int pipesnum)
{
	int i, fd[2], sts, childpid;
	int auxpid = getpid();

	for(i=0; i < pipesnum-1; i++){
		pipe(fd);
		cml->pipes[i][0] = fd[0];
		cml->pipes[i][1] = fd[1];
	}

	for(i=0; i < pipesnum; i++){
		childpid = runpipeline(cml, i, pipesnum);
	}
	for(i=0; i < pipesnum; i++){
		if(nowait){
			if(auxpid != childpid){
				wait(&sts);
			}
		}else{
			wait(&sts);
		}
	}
}

static void
inandout(int nowait, commands *cml, int pipesnum)
{
	char *input;
	char *output;
	char *args[20];

	if(nowait){
		cml->stdin = open("/dev/null", O_RDONLY);
	}else{
		cml->stdin = 0;
	}
	cml->stdout = 1;

	input = strpbrk(cml->piped[pipesnum-1], "<");
	output = strpbrk(cml->piped[pipesnum-1], ">");

	if(input != NULL && output != NULL){
		input++;
		output++;
		argsdivider(output, args, "< \n\r\t");
		output = args[0];
		argsdivider(input, args, "> \n\r\t");
		input = args[0];
		cml->stdin = open(input, O_RDONLY);
		if(cml->stdin < 0){
			err(1,"open");
		}
		cml->stdout = open(output, O_WRONLY|O_CREAT|O_TRUNC, 0644);
		if(cml->stdout<0){
			err(1, "open");
		}
	}else if(input == NULL && output != NULL){
		output++;
		argsdivider(output, args, "< \n\r\t");
		output = args[0];
		cml->stdout = open(output, O_WRONLY|O_CREAT|O_TRUNC);
		if(cml->stdout<0){
			err(1, "open");
		}
	}else if(output == NULL && input != NULL){
		input++;
		argsdivider(input, args, "> \n\r\t");
		input = args[0];
		cml->stdin = open(input, O_RDONLY);
		if(cml->stdin < 0){
			err(1,"open");
		}
	}
}

int
main (int argc, char *argv[])
{
	int size = 256;
	int pipesnum, auxnum, factores, nowait;
	int maxargs = 20;
	char *args[maxargs];
	char *aux[maxargs];
	char *last_char;
	char *command = "\0";
	char string[size];
	char *auxstr;
	commands *cml = (commands*) malloc (sizeof(commands));

	while(strcmp(command,"exit")){
		usleep(3500);
		factores = 0;
		nowait = 0;

		printf("; ");
		fgets(string, size, stdin);

		//Generamos copia
		auxstr = (char*) malloc (strlen(string)+1);
		strcpy(auxstr, string);
		auxnum = argsdivider(auxstr, aux, " \n\r\t");
		command = aux[0];

		//Parsing de los comandos "piped"
		pipesnum = argsdivider(string, args, "|");
		cml->piped = args;

		//Comprobamos si hay &
		last_char = cml->piped[pipesnum-1];
		last_char = last_char + strlen(cml->piped[pipesnum-1])-2;
		argsdivider(last_char, aux, "\n");
		last_char = aux[0];
		if(!strcmp(last_char, "&")){
			nowait = 1;
		}

		//Obtenemos la salida y entrada estandar
		inandout(nowait, cml, pipesnum);

		//Ejecutamos los procesos
		//Preparar ultimo comando
		argsdivider(cml->piped[pipesnum-1], aux, "><&");
		cml->piped[pipesnum-1] = aux[0];

		if(strcmp(command, "exit")){
			if(!strcmp(command, "cd")){
				if(auxnum <= 2){
					change_dir(auxnum, aux);
				}else{
					errx(1, "cd error");
				}
			}else{
				if(auxnum == 1){
					factores = argsdivider(auxstr, aux, "=");
				}
				if(factores == 2){
					assign_vars(aux);
				}else{
					pipelining(nowait, cml, pipesnum);
				}
			}
		}
	
		if(cml->stdin != 0){
			close(cml->stdin);
		}
		if(cml->stdout != 1){
			close(cml->stdout);
		}
	}
	free(cml);
	free(auxstr);
	exit(0);
}
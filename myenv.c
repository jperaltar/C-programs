#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


static void
variables_name (int num_var, char **vars)
{
	int i;
	char *cont;
	
	for (i = 1; i < num_var; i++){
		cont = getenv (vars[i]);
		if (cont != NULL){
			printf ("%s: %s\n", vars[i], cont);
		}else {
			fprintf (stderr, "error: %s does not exists\n", vars[i]);
		}
	}
}

int
main (int argc, char *argv[])
{
	pid_t PID = getpid();
	uid_t UID = getuid();
	
	printf ("PID: %d\n", PID);
	printf ("UID: %d\n", UID);
	variables_name (argc, argv);
	
	exit(0);
}
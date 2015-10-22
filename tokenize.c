#include <stdlib.h>
#include <stdio.h>

char divid[] = {' ', '\t', '\n', '\r'};
int divelemnum = (sizeof(divid) / sizeof(*divid));

int
isdivider(char c)
{
	int divider = 0;
	int i = 0;
	
	for (i = 0; i < divelemnum && !(divider); i ++){ 
		if (c == divid[i]){
			divider = 1;
		}
	}
	return divider;
}
	
int 
mytokenize(char *str, char **args, int maxargs)
{
	int i = 0;
	int word = 0;
	int substringsnum = 0;
	int endofstring = 0;
	
	while (!endofstring && substringsnum < maxargs){
		printf("%c\n", str[i]);
		if (isdivider(str[i])){
			printf("Es separador\n");
			if (word){
				str[i] = '\0';
				/* Si estabamos recorriendo una palabra 
					aumentamos en uno el contador de cadenas */
				substringsnum ++;
			}
			word = 0;
		}else if (str[i] == '\0'){
			printf("Final de la cadena\n");
			if (word){
				substringsnum ++;
			}
			endofstring = 1;
		}else {
			printf("No es separador\n");
			if (!word){
				args[substringsnum] = str + i;
			}
			/* Variable que nos indica si estamos o no
				actualmente recorriendo una palabra */
			word = 1;
		}
		i ++;
	}
	return substringsnum;
}

int
main(int argc, char *argv[])
{
	char string1[] = "hola 		tío soy		\nun  string \r.";
	int maxsubstrings1 = 6;
	int i = 0;
	char *substrings1[maxsubstrings1];

	mytokenize (string1,substrings1,maxsubstrings1);
	printf ("\n");
	printf ("División en segmentos de la cadena:\n");
	for (i=0; i < maxsubstrings1; i ++){
		printf ("%s\n", substrings1[i]);
	}
	exit(EXIT_SUCCESS);
}
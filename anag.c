#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct list{
	char *word;
	struct list *next; 
	};

typedef struct list list;

/*Función que cuenta el número de veces que se
repite una misma letra en una palabra*/
static int
timeschar (char character, char *str)
{
	int times = 0;
	int i = 0;
	
	for(i = 0; i < strlen (str); i ++){
		if (character == str[i]){
			times ++;
		}
	}
	return times;
}

/*Función que indica si dos palabras son anagramas
entre si*/
static int
isanagram (char *str1, char *str2)
{
	int anagram = 0;
	int samechar1 = 0;
	int samechar2 = 0;
	int sizestr1 = strlen (str1);
	int sizestr2 = strlen (str2);
	int i;
	
	if (sizestr1 == sizestr2){
		for (i = 0; i < sizestr1 && 
				(samechar1 == samechar2); i ++){
			samechar1 = timeschar (str1[i], str1);
			samechar2 = timeschar (str1[i], str2);
			if (samechar1 == samechar2){
				anagram = 1;
			}else {
				anagram = 0;
			}
		}
	}
	return anagram;
}

/*Función que libera la memoria de los punteros de la
lista enlazada*/
static void
freelist (list *anagramlist)
{
	list *aux;
	while (anagramlist->next != NULL){
		aux = anagramlist;
		anagramlist = anagramlist->next;
		aux->next = NULL;
		free(aux);
	}
}

/*Función que compara en cada palabra de la lista enlazada
si las letras están en la misma posición*/
static int
samepos_list (int letter, list *aux, list *anagramlist)
{
	int samepos = 1;
	aux = anagramlist;
	while (aux != NULL && samepos){
		if (anagramlist->word[letter] != aux->word[letter]){
			samepos = 0;
		}
		aux = aux->next;
	}
	return samepos;
}

/*Función que escribe las letras que están en la misma
posición en todas las palabras*/
static void
samepositions (list *anagramlist)
{
	int sizestr = strlen (anagramlist->word);
	int i;
	list *aux = anagramlist->next;

	printf ("[");
	for (i = 0; i < sizestr; i ++){
		if (samepos_list (i, aux, anagramlist)){
			printf ("%c", anagramlist->word[i]);	
		}
	}
	printf ("]\n");
}

/*Función que imprime la lista enlazada*/
static void
printlist (list *anagramlist)
{
	list *aux = anagramlist;

	while (aux != NULL){
		printf ("%s ", aux->word);
		aux = aux->next;
	}
}

static void
comparison (int listcounter, int index, 
			int wordsnum, char *word, char *string[])
{
	int j;
	char *nullstring = "\0";
	list *new = (list *) malloc (sizeof(list));
	new->word = word;
	new->next = NULL;
	list *aux = new;

	/*Inicializamos j a index + 1 para que no se repitan 
	comprobaciones anteriores */
	for (j = index + 1; j < wordsnum; j++){
		if (j != index && string[j] != nullstring){
			if (isanagram(word, string[j])){
				aux->next = (list *) malloc (sizeof(list));
				aux = aux->next;
				aux->word = string[j];
				aux->next = NULL;
				string[j] = nullstring;
				listcounter ++;
			}
		}
	}
	if (listcounter > 1){
		printlist (new);
		samepositions (new);
		freelist (new);
	}
}

static void
comparisonlist (int wordsnum, char *string[])
{
	int i;
	int listcounter;
	char *nullstring = "\0";

	for (i = 1; i < wordsnum; i++){
		if (string[i] != nullstring){
			listcounter = 1;
			comparison (listcounter, i, 
						wordsnum, string[i], string);
			string[i] = nullstring;
		}
	}
}

int
main (int argc, char *argv[])
{
	if (argc > 2){
		comparisonlist (argc, argv);
		exit(0);
	}else {
		printf ("Numero de argumentos insuficiente\n");
		exit(1);
	}
}
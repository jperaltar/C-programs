#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

struct coord{
	int x;
	int y;
};

struct coord_list{
	struct coord coord;
	struct coord_list *next;
};

typedef struct coord coord;
typedef struct coord_list coord_list;

static coord_list *
go_over_list (coord_list *aux, coord_list *limit)
{
	while (aux->next != limit){
		aux = aux->next;
	}
	return aux;
}

static void
free_list (coord_list *list)
{
	coord_list *last;
	coord_list *aux;
	coord_list *null = NULL;

	while (list->next != NULL){
		last = list;
		aux = list;
		last = go_over_list (last, null);
		aux = go_over_list (aux, last);
		free (last);
		aux->next = NULL;
	}
	free (list);
	list = NULL;
}

static void
print_coord_list (coord_list *list)
{
	coord_list *aux = list;

	while (aux != NULL){
		list->coord = aux->coord;
		printf ("(%d,%d)\n", list->coord.x, list->coord.y);
		aux = aux->next;
	}
}

static coord_list *
add_coord (coord new_coord, coord_list *list)
{
	coord_list *aux = NULL;
	coord_list *new = (coord_list *) malloc (sizeof(coord_list));
	new->coord = new_coord;
	new->next = NULL;
	aux = new;
	
	if (list == NULL){
		list = aux;
	}else {
		aux->next = list;
		list = aux;
	}
	return list;
}

static coord_list *
create_list (coord_list *list, coord *buf, int bytes_num)
{
	int Num_coords;
	int Size_coord = sizeof(coord);
	int i;
	
	Num_coords = bytes_num / Size_coord;
	/*Lista enlazada con las coordenadas del fichero*/
	for (i = 0; i < Num_coords; i++){
		list = add_coord (buf[i], list);
	}
	return list;
}

static coord_list *
read_coord_file (int fd, coord *buf, int sizeofbuf, coord_list *list)
{
	int nr = 0;
	int Size_coord = sizeof(coord);
	int module = sizeofbuf % Size_coord;

	if (module != 0){
		sizeofbuf = sizeofbuf - module;
	}

	for (;;){
		nr = read (fd, buf, sizeofbuf);
		if (nr > 0){
			list = create_list (list, buf, nr);
		}else if (nr == 0){
			break;
		}else if (nr < 0){
			err (1, "read");
		}
	}
	return list;
}

static void
print_equal_coords (int fd, int Num_coords)
{	
	int i;
	coord aux;
	
	for (i = 0; i <= Num_coords; i++){
		aux.x = i;
		aux.y = i;
		write (fd, &aux, sizeof(coord));
	}
}

int
main (int argc, char *argv[])
{
	int Num_coords = 0;
	int wfd = 1;
	int rfd = 0;
	int sizeofbuf = 30;
	char *W_Option = "-w";
	char *First_arg = argv[1];
	coord buf[sizeofbuf];
	coord_list *list = NULL;

	
	if (argc > 1 && argc <= 3){
		if (!strcmp(First_arg, W_Option)){
			Num_coords = atoi(argv[2]);
			print_equal_coords (wfd, Num_coords);
		}else {
			rfd = open(First_arg, O_RDONLY);
			list = read_coord_file (rfd, buf, sizeofbuf, list);
			close (rfd);
		}
	}else if (argc == 1){
		rfd = 0;
		list = read_coord_file (rfd, buf, sizeofbuf, list);
	}else if (argc > 3){
		err (1, "Número excesivo de argumentos");
	}
	print_coord_list (list);
	free_list (list);
	exit(0);
}

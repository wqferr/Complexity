#ifndef COORD_LIST_H
#define COORD_LIST_H 1

#include <stddef.h>

typedef struct coord_list coord_list;
typedef struct coord coord;

struct coord {
	size_t x;
	size_t y;
};

coord_list *clist_create(size_t initial_capacity);
void clist_destroy(coord_list *clist);

int clist_add(coord_list *clist, coord c);

const coord *clist_as_array(const coord_list *clist, size_t *size);

#endif
#include "struct/coordlist.h"

#include <stdlib.h>

struct coord_list {
	coord *coords;

	size_t size;
	size_t capacity;
};

inline void _clist_ensure_capacity(coord_list *clist, size_t capacity);


coord_list *clist_create(size_t initial_capacity) {
	coord_list *clist = malloc(sizeof(*clist));
	clist->coords = NULL;
	clist->size = 0;
	clist->capacity = 0;
	_clist_ensure_capacity(clist, initial_capacity);

	return clist;
}

void clist_destroy(coord_list *clist) {
	free(clist->coords);
	free(clist);
}

int clist_add(coord_list *clist, coord c) {
	if (clist->capacity == clist->size) {
		_clist_ensure_capacity(clist, 2*clist->capacity);
	}
	clist->coords[clist->size] = c;
	clist->size++;

	return 0;
}

const coord *clist_as_array(const coord_list *clist, size_t *size) {
	*size = clist->size;
	return clist->coords;
}

void _clist_ensure_capacity(coord_list *clist, size_t capacity) {
	if (capacity > clist->capacity) {
		clist->coords = realloc(
			clist->coords,
			capacity * sizeof(*clist->coords));
		clist->capacity = capacity;
	}
}
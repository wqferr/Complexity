#include "core/transform.h"

#include <stdlib.h>

#include "struct/coordlist.h"

#define OUTPUT_MAP_INITIAL_CAP 5


#include <stdio.h>

rgba_image *warp(const rgba_image *input, warp_f transformation) {
	/* Coordinates for scanning input image */
	double x0, y0;
	double complex z0;

	/* Coordinates for mapping into outpub image */
	double x1, y1;
	double complex z1;

	size_t i0, j0;
	size_t i1, j1;
	size_t width, height;

	coord_list ***mapping;
	coord c0;

	rgbaimg_get_dimensions(input, &width, &height);
	mapping = malloc(height * sizeof(*mapping));
	for (i0 = 0; i0 < height; i0++) {
		mapping[i0] = malloc(width * sizeof(**mapping));
		for (j0 = 0; j0 < width; j0++) {
			mapping[i0][j0] = clist_create(OUTPUT_MAP_INITIAL_CAP);
		}
	}

	for (i0 = 0; i0 < height; i0++) {
		y0 = (double) (height-1 - i0) / (height-1);
		for (j0 = 0; j0 < width; j0++) {
			x0 = (double) j0 / (width-1);

			z0 = x0 + 1j*y0;

			z1 = transformation(z0);
			x1 = creal(z1);
			y1 = cimag(z1);

			if (0 <= x1 && x1 <= 1 && 0 <= y1 && y1 <= 1) {
				i1 = (height-1) - y1 * (height-1);
				j1 = x1 * (width-1);

				c0.x = j0;
				c0.y = i0;
				clist_add(mapping[i1][j1], c0);
			}
		}
	}

	// TODO reduce values in mapping matrix to a single color

	for (i0 = 0; i0 < height; i0++) {
		for (j0 = 0; j0 < width; j0++) {
			clist_destroy(mapping[i0][j0]);
		}
		free(mapping[i0]);
	}
	free(mapping);

	return NULL;
}

#include "core/transform.h"

#include <stdlib.h>

#include "struct/coordlist.h"

#define OUTPUT_MAP_INITIAL_CAP 5


#include <stdio.h>

void _set_out_pixel(
	const rgba_image *input, rgba_image *output,
	size_t x, size_t y,
	coord_list *clist) {

	uint32_t sum_r = 0;
	uint32_t sum_g = 0;
	uint32_t sum_b = 0;
	uint32_t sum_a = 0;

	size_t len;
	size_t i;
	const coord *coord_arr;
	coord c;
	rgba_pixel in_pixel;
	rgba_pixel out_pixel;

	if (clist != NULL) {
		coord_arr = clist_as_array(clist, &len);

		for (i = 0; i < len; i++) {
			c = coord_arr[i];
			if (rgbaimg_get_pixel(input, c.x, c.y, &in_pixel) == 0) {
				sum_r += in_pixel.r * in_pixel.a;
				sum_g += in_pixel.g * in_pixel.a;
				sum_b += in_pixel.b * in_pixel.a;
				sum_a += in_pixel.a;
			}
		}
	}

	if (sum_a > 0) {
		out_pixel.r = sum_r / sum_a;
		out_pixel.g = sum_g / sum_a;
		out_pixel.b = sum_b / sum_a;
		if (sum_a > UINT8_MAX) {
			sum_a = UINT8_MAX;
		}
		out_pixel.a = sum_a;
	} else {
		out_pixel.r = 0;
		out_pixel.g = 0;
		out_pixel.b = 0;
		out_pixel.a = 0;
	}

	rgbaimg_set_pixel(output, x, y, out_pixel);
}

rgba_image *warp(const rgba_image *input, warp_f transformation) {
	/* Coordinates for scanning input image */
	double x0, y0;
	double complex z0;

	/* Coordinates for mapping into output image */
	double x1, y1;
	double complex z1;

	size_t i0, j0;
	size_t i1, j1;
	size_t width, height;

	coord c0;
	coord_list ***mapping;

	rgba_image *output;

	rgbaimg_get_dimensions(input, &width, &height);
	mapping = calloc(height, sizeof(*mapping));

	for (i0 = 0; i0 < height; i0++) {
		/* Invert y axis */
		y0 = (double) (height-1 - i0) / (height-1);
		for (j0 = 0; j0 < width; j0++) {
			x0 = (double) j0 / (width-1);

			/* Transform coordinates into a complex number */
			z0 = x0 + 1j*y0;

			/* Apply function */
			z1 = transformation(z0);
			x1 = creal(z1);
			y1 = cimag(z1);

			/* Output within range */
			if (0 <= x1 && x1 <= 1 && 0 <= y1 && y1 <= 1) {
				/* Round to nearest integer */
				i1 = 0.5 + (height-1) - y1 * (height-1);
				j1 = 0.5 + x1 * (width-1);


				/* Create coordinate list if it doesn't exist */
				if (mapping[i1] == NULL) {
					mapping[i1] = calloc(width, sizeof(**mapping));
				}
				if (mapping[i1][j1] == NULL) {
					mapping[i1][j1] = clist_create(OUTPUT_MAP_INITIAL_CAP);
				}

				c0.x = j0;
				c0.y = i0;
				clist_add(mapping[i1][j1], c0);
			}
		}
	}

	/* Set output pixels and destroy coordinate lists */
	output = rgbaimg_create(width, height);
	for (i0 = 0; i0 < height; i0++) {
		for (j0 = 0; j0 < width; j0++) {
			_set_out_pixel(input, output, j0, i0, mapping[i0][j0]);

			if (mapping[i0][j0] != NULL) {
				clist_destroy(mapping[i0][j0]);
			}
		}
		free(mapping[i0]);
	}
	free(mapping);

	return output;
}

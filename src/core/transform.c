#include "core/transform.h"

#include <stdlib.h>
#include <omp.h>

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
	return warp_ext(
		input, transformation, NULL,
		0+0j, 1+1j,
		0+0j, 1+1j,
		0, 0);
}

rgba_image *warp_ext(
	const rgba_image *input, warp_f transformation, const void *arg,
	double complex min_in, double complex max_in,
	double complex min_out, double complex max_out,
	size_t out_width, size_t out_height) {

	double min_x_in = creal(min_in);
	double max_x_in = creal(max_in);
	double min_y_in = cimag(min_in);
	double max_y_in = cimag(max_in);

	double min_x_out = creal(min_out);
	double max_x_out = creal(max_out);
	double min_y_out = cimag(min_out);
	double max_y_out = cimag(max_out);

	/* Coordinates for scanning input image */
	double x0, y0;
	double complex z0;

	/* Coordinates for mapping into output image */
	double x1, y1;
	double complex z1;
	
	coord c0;

	size_t i0, j0;
	size_t i1, j1;
	size_t in_width, in_height;

	/*
	 * A matrix of lists such that, if
	 * 	(i0, j0) \in mapping[i1][j1]
	 * then the complex numbers z0, corresponding to (j0, i0) in the input
	 * space, and z1, corresponding to (j1, i1), satisfy:
	 *  z1 = transformation(z0)
	 *  
	 * This is used later on to determine the color of a given pixel
	 * in the output image given the coordinates which hit it.
	 */
	coord_list ***mapping;

	rgba_image *output;

	rgbaimg_get_dimensions(input, &in_width, &in_height);

	if (out_width == 0) {
		out_width = in_width;
	}
	if (out_height == 0) {
		out_height = in_height;
	}

	/* All mapping rows set to NULL until it is hit */
	mapping = calloc(out_height, sizeof(*mapping));

	/* Loop through a lattice in the input space */
	#pragma omp parallel for\
				num_threads(4)\
				private(x0, y0, z0, x1, y1, z1, c0, j0, i1, j1)\
				schedule(static)
	for (i0 = 0; i0 < in_height; i0++) {
		/* Invert y axis and map to [0, 1] */
		y0 = (double) (in_height-1 - i0) / (in_height-1);
		/* Map from [0, 1] to [min_y_in, max_y_in] */
		y0 = y0 * max_y_in + (1-y0) * min_y_in;

		for (j0 = 0; j0 < in_width; j0++) {
			/* Map to [0, 1] */
			x0 = (double) j0 / (in_width-1);
			/* Map from [0, 1] to [min_x_in, max_x_in] */
			x0 = x0 * max_x_in + (1-x0) * min_x_in;

			/* Transform coordinates into a complex number */
			z0 = x0 + 1j*y0;

			/* Apply function */
			z1 = transformation(z0, arg);
			x1 = creal(z1);
			y1 = cimag(z1);

			x1 = (x1-min_x_out) / (max_x_out-min_x_out);
			y1 = (y1-min_y_out) / (max_y_out-min_y_out);

			/* Output within range */
			if (0 <= x1 && x1 <= 1 && 0 <= y1 && y1 <= 1) {
				/* Round to nearest integer */
				i1 = 0.5 + (out_height-1) - y1 * (out_height-1);
				j1 = 0.5 + x1 * (out_width-1);

				c0.x = j0;
				c0.y = i0;

				/* Create coordinate list if it doesn't exist */
				#pragma omp critical
				{
					if (mapping[i1] == NULL) {
						mapping[i1] = calloc(out_width, sizeof(**mapping));
					}
				}
				#pragma omp critical
				{
					if (mapping[i1][j1] == NULL) {
						mapping[i1][j1] = clist_create(OUTPUT_MAP_INITIAL_CAP);
					}
				}

				#pragma omp critical
				{
					clist_add(mapping[i1][j1], c0);
				}
			}
		}
	}

	/* Set output pixels and destroy coordinate lists */
	output = rgbaimg_create(out_width, out_height);
	for (i0 = 0; i0 < out_height; i0++) {
		if (mapping[i0] != NULL) {
			for (j0 = 0; j0 < out_width; j0++) {
				if (mapping[i0][j0] != NULL) {
					_set_out_pixel(input, output, j0, i0, mapping[i0][j0]);
					clist_destroy(mapping[i0][j0]);
				}
			}
			free(mapping[i0]);
		}
	}
	free(mapping);

	return output;
}

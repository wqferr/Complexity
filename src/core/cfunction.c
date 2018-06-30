#include "core/cfunction.h"

#include <stdlib.h>
#include <omp.h>
#include <math.h>

#include "util/pi.h"

#include "struct/coordlist.h"

#define OUTPUT_MAP_INITIAL_CAP 5


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


double complex f_identity(double complex z, const void *arg) {
	(void) arg;
	return z;
}


double complex f_null(double complex z, const void *arg) {
	(void) z;
	(void) arg;
	return 0;
}


double complex coord_to_complex(
		const rgba_image *img,
		size_t inx, size_t iny,
		double complex min, double complex max) {
	size_t w, h;
	double tx, ty;
	double x, y;
	double x_min = creal(min);
	double x_max = creal(max);
	double y_min = cimag(min);
	double y_max = cimag(max);

	rgbaimg_get_dimensions(img, &w, &h);
	tx = (double) inx / w;
	ty = 1 - (double) iny / h;

	y = (1-ty)*y_min + ty*y_max;
	x = (1-tx)*x_min + tx*x_max;
	return x + y*1.0i;
}

bool complex_to_coord(
		const rgba_image *img,
		double complex z,
		double complex min, double complex max,
		size_t *outx, size_t *outy) {
	size_t w, h;
	double tx, ty;
	double x = creal(z);
	double y = cimag(z);
	double x_min = creal(min);
	double x_max = creal(max);
	double y_min = cimag(min);
	double y_max = cimag(max);

	rgbaimg_get_dimensions(img, &w, &h);
	tx = (x - x_min) / (x_max - x_min);
	ty = (y - y_min) / (y_max - y_min);

	if (tx < 0 || ty < 0 || tx >= 1 || ty >= 1) {
		return false;
	}
	*outx = (size_t) (0.5 + (tx*w));
	*outy = (size_t) (0.5 + ((1-ty) * h));

	return true;
}


void complex_to_color(
		double complex z,
		float *hue,
		float *value) {
	*hue = 360.0f * (carg(z) / (2*M_PI));
	*hue = fmodf(*hue + 360.0f, 360.0f);
	*value = 1-exp(-cabs(z));
}


rgba_image *warp(const rgba_image *input, complex_f transformation) {
	return warp_ext(
		input, transformation, NULL,
		0+0.0j, 1+1.0j,
		0+0.0j, 1+1.0j,
		0, 0);
}

rgba_image *warp_ext(
	const rgba_image *input, complex_f transformation, const void *arg,
	double complex min_in, double complex max_in,
	double complex min_out, double complex max_out,
	size_t out_width, size_t out_height) {
	/* Coordinates for scanning input image */
	double complex z0;
	/* Coordinates for mapping into output image */
	double complex z1;
	
	coord c0;

	size_t idx;
	size_t i0, j0;
	size_t i1, j1;
	size_t in_width, in_height;
	size_t in_n_pixels;

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
	output = rgbaimg_create(out_width, out_height);

	in_n_pixels = in_width * in_height;

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
				private(z0, z1, c0, i0, j0, i1, j1)\
				schedule(static)
	for (idx = 0; idx < in_n_pixels; idx++) {
		i0 = idx / in_width;
		j0 = idx % in_width;

		z0 = coord_to_complex(input, j0, i0, min_in, max_in);

		/* Apply function */
		z1 = transformation(z0, arg);
		if (complex_to_coord(output, z1, min_out, max_out, &j1, &i1)) {
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

	/* Set output pixels and destroy coordinate lists */
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


void imprint(rgba_image *canvas, color_f color) {
	imprint_ext(
		canvas,
		(-1-1.0i), (+1+1.0i),
		color, NULL);
}

void imprint_ext(
		rgba_image *canvas,
		double complex min, double complex max,
		color_f color, const void *arg) {
	size_t width, height;
	size_t i, j;
	double complex z;
	double alpha;
	rgba_pixel cur_pixel, new_pixel;

	rgbaimg_get_dimensions(canvas, &width, &height);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			z = coord_to_complex(canvas, j, i, min, max);
			rgbaimg_get_pixel(canvas, j, i, &cur_pixel);
			new_pixel = cur_pixel;
			color(&new_pixel, z, arg);
			alpha = (double) new_pixel.a / UINT8_MAX;
			new_pixel.r = alpha * new_pixel.r + (1-alpha) * cur_pixel.r;
			new_pixel.g = alpha * new_pixel.g + (1-alpha) * cur_pixel.g;
			new_pixel.b = alpha * new_pixel.b + (1-alpha) * cur_pixel.b;
			new_pixel.a = cur_pixel.a;
			rgbaimg_set_pixel(canvas, j, i, new_pixel);
		}
	}
}
#ifndef CFUNCTION_H
#define CFUNCTION_H 1

#include <complex.h>
#include <stdbool.h>

#include "img/rgbaimg.h"

typedef double complex (*complex_f)(double complex z, const void *arg);
typedef rgba_pixel (*color_f)(double complex z, const void *arg);

double complex coord_to_complex(
	const rgba_image *img,
	size_t x, size_t y,
	double complex min, double complex max);
bool complex_to_coord(
	const rgba_image *img,
	double complex z,
	double complex min, double complex max,
	size_t *x, size_t *y);

rgba_image *warp(const rgba_image *input, complex_f transformation);
rgba_image *warp_ext(
	const rgba_image *input,
	complex_f transformation, const void *arg,
	double complex min_in, double complex max_in,
	double complex min_out, double complex max_out,
	size_t out_width, size_t out_height);

void imprint(rgba_image *canvas, color_f color);
void imprint_ext(
	rgba_image *canvas,
	color_f color, const void *arg,
	double complex min, double complex max);

#endif
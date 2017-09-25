#ifndef TRANSFORM_H
#define TRANSFORM_H 1

#include <complex.h>

#include "img/rgbaimg.h"

typedef double complex (*warp_f)(double complex z);

rgba_image *warp(const rgba_image *input, warp_f transformation);

rgba_image *warp_ext(
	const rgba_image *input,
	warp_f transofrmation,
	double min_x_in, double min_y_in,
	double max_x_in, double max_y_in,
	double min_x_out, double min_y_out,
	double max_x_out, double max_y_out,
	size_t out_width, size_t out_height);

#endif
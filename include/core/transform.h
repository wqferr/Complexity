#ifndef TRANSFORM_H
#define TRANSFORM_H 1

#include <complex.h>

#include "img/rgbaimg.h"

typedef double complex (*warp_f)(double complex z);

rgba_image *warp(const rgba_image *input, warp_f transformation);

rgba_image *warp_ext(
	const rgba_image *input,
	warp_f transofrmation,
	double complex min_in, double complex max_in,
	double complex min_out, double complex max_out,
	size_t out_width, size_t out_height);

#endif
#ifndef IMPRINT_H
#define IMPRINT_H

#include <complex.h>

#include "img/rgbaimg.h"
#include "core/cfunction.h"

void imprint_line(
    rgba_image *canvas, double complex min, double complex max,
    rgba_pixel color,
    double x_coeff, double y_coeff, double indep_coeff, double width);
void imprint_rect(
    rgba_image *canvas, double complex min, double complex max,
    rgba_pixel color,
    double complex rect_min, double complex rect_max);
void imprint_circle(
    rgba_image *canvas, double complex min, double complex max,
    rgba_pixel color,
    double complex center, double min_radius, double max_radius,
    double arc_start, double arc_end);
void imprint_line_segment(
    rgba_image *canvas,
    double complex min, double complex max,
    rgba_pixel color,
    double x_coeff, double y_coeff,
    double complex center,
    double width, double length);
void imprint_function_as_hsv(
    rgba_image *canvas,
    double complex min, double complex max,
    complex_f f, const void *arg);

#endif
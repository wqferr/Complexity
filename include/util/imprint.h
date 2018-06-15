#ifndef IMPRINT_H
#define IMPRINT_H

#include <complex.h>

#include "img/rgbaimg.h"
#include "core/cfunction.h"

void imprint_line(
    rgba_image *canvas, double complex min, double complex max,
    rgba_pixel color, double margin,
    double x_coeff, double y_coeff, double indep_coeff);

#endif
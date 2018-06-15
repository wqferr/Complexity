#include "util/imprint.h"

#include <math.h>

typedef struct {
    double x_coeff;
    double y_coeff;
    double indep_coeff;

    double margin;
    rgba_pixel color;
    
    double complex min;
    double complex max;
} imprint_line_data;

rgba_pixel _do_imprint_line(double complex z, const void *arg) {
    imprint_line_data data = *((const imprint_line_data *) arg);
    rgba_pixel out = data.color;
    double x = creal(z);
    double y = cimag(z);

    out.a = 0;

    if (fabs(x*data.x_coeff + y*data.y_coeff + data.indep_coeff) < data.margin) {
        out.a = 255;
    }
    return out;
}

void imprint_line(
        rgba_image *canvas, double complex min, double complex max,
        rgba_pixel color, double margin,
        double x_coeff, double y_coeff, double indep_coeff) {
    imprint_line_data data = {
        .x_coeff = x_coeff,
        .y_coeff = y_coeff,
        .indep_coeff = indep_coeff,

        .margin = margin,
        .color = color,

        .min = min,
        .max = max
    };
    imprint_ext(canvas, min, max, &_do_imprint_line, &data);
}
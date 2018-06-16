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

void _do_imprint_line(rgba_pixel *out, double complex z, const void *arg) {
    imprint_line_data data = *((const imprint_line_data *) arg);
    double x = creal(z);
    double y = cimag(z);
    double dist_to_line = fabs(x*data.x_coeff + y*data.y_coeff + data.indep_coeff);
    dist_to_line /= sqrt(data.x_coeff*data.x_coeff + data.y_coeff*data.y_coeff);

    if (dist_to_line < data.margin) {
        *out = data.color;
    }
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


typedef struct {
    double complex rect_min;
    double complex rect_max;

    rgba_pixel color;

    double complex min;
    double complex max;
} imprint_rect_data;

void _do_imprint_rect(rgba_pixel *out, complex double z, const void *arg) {
    imprint_rect_data data = *((const imprint_rect_data *) arg);
    double x = creal(z);
    double y = cimag(z);

    if (x >= creal(data.rect_min) && x <= creal(data.rect_max)
            && y >= cimag(data.rect_min) && y <= cimag(data.rect_max)) {
        *out = data.color;
    }
}

void imprint_rect(
        rgba_image *canvas, double complex min, double complex max,
        rgba_pixel color,
        double complex rect_min, double complex rect_max) {
    imprint_rect_data data = {
        .rect_min = rect_min,
        .rect_max = rect_max,

        .color = color,

        .min = min,
        .max = max
    };
    imprint_ext(canvas, min, max, &_do_imprint_rect, &data);
}
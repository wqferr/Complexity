#include "util/imprint.h"

#include <math.h>

#include "core/cfunction.h"
#include "img/hsv.h"

typedef struct {
    double x_coeff;
    double y_coeff;
    double indep_coeff;

    double width;
    rgba_pixel color;
} imprint_line_data;

void _do_imprint_line(rgba_pixel *out, double complex z, const void *arg) {
    imprint_line_data data = *((const imprint_line_data *) arg);
    double x = creal(z);
    double y = cimag(z);
    double dist_to_line = fabs(x*data.x_coeff + y*data.y_coeff + data.indep_coeff);
    dist_to_line /= sqrt(data.x_coeff*data.x_coeff + data.y_coeff*data.y_coeff);

    if (dist_to_line < data.width/2) {
        *out = data.color;
    }
}

void imprint_line(
        rgba_image *canvas, double complex min, double complex max,
        rgba_pixel color,
        double x_coeff, double y_coeff, double indep_coeff, double width) {
    imprint_line_data data = {
        .x_coeff = x_coeff,
        .y_coeff = y_coeff,
        .indep_coeff = indep_coeff,

        .width = width,
        .color = color,
    };
    imprint_ext(canvas, min, max, &_do_imprint_line, &data);
}


typedef struct {
    double complex rect_min;
    double complex rect_max;

    rgba_pixel color;
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
    };
    imprint_ext(canvas, min, max, &_do_imprint_rect, &data);
}


typedef struct {
    double complex center;
    double min_radius;
    double max_radius;
    double arc_start;
    double arc_end;

    rgba_pixel color;
} imprint_circle_data;

void _do_imprint_circle(rgba_pixel *out, double complex z, const void *arg) {
    imprint_circle_data data = *((const imprint_circle_data *) arg);
    double x = creal(z);
    double y = cimag(z);
    double cx = creal(data.center);
    double cy = cimag(data.center);
    double dx = x - cx;
    double dy = y - cy;
    double dist_to_center = sqrt(dx*dx + dy*dy);
    double angle = atan2(dy, dx);

    if (dist_to_center >= data.min_radius && dist_to_center <= data.max_radius
            && (angle >= data.arc_start && angle <= data.arc_end)) {
        *out = data.color;
    }
}

void imprint_circle(
        rgba_image *canvas, double complex min, double complex max,
        rgba_pixel color,
        double complex center, double min_radius, double max_radius,
        double arc_start, double arc_end) {
    imprint_circle_data data = {
        .center = center,
        .min_radius = min_radius,
        .max_radius = max_radius,
        .arc_start = arc_start,
        .arc_end = arc_end,
        .color = color
    };
    imprint_ext(canvas, min, max, &_do_imprint_circle, &data);
}

typedef struct {
    double x_coeff_main_axis;
    double y_coeff_main_axis;
    double indep_coeff_main_axis;

    double x_coeff_coaxis;
    double y_coeff_coaxis;
    double indep_coeff_coaxis;

    double length;
    double width;

    double complex center;

    rgba_pixel color;
} imprint_line_segment_data;


void _do_imprint_line_segment(rgba_pixel *out, double complex z, const void *arg) {
    imprint_line_segment_data data = *((const imprint_line_segment_data *) arg);
    double x = creal(z);
    double y = cimag(z);
    double dist_to_main_axis = fabs(
        x*data.x_coeff_main_axis + y*data.y_coeff_main_axis + data.indep_coeff_main_axis);
    double dist_to_coaxis = fabs(
        x*data.x_coeff_coaxis + y*data.y_coeff_coaxis + data.indep_coeff_coaxis);

    dist_to_main_axis /= sqrt(
        data.x_coeff_main_axis*data.x_coeff_main_axis + data.y_coeff_main_axis*data.y_coeff_main_axis);
    dist_to_coaxis /= sqrt(
        data.x_coeff_coaxis*data.x_coeff_coaxis + data.y_coeff_coaxis*data.y_coeff_coaxis);

    if (dist_to_main_axis <= data.width/2 && dist_to_coaxis <= data.length/2) {
        *out = data.color;
    }
}

void imprint_line_segment(
        rgba_image *canvas,
        double complex min, double complex max,
        rgba_pixel color,
        double x_coeff, double y_coeff,
        double complex center,
        double width, double length) {
    imprint_line_segment_data data = {
        .x_coeff_main_axis = x_coeff,
        .y_coeff_main_axis = y_coeff,
        .indep_coeff_main_axis = -(x_coeff*creal(center) + y_coeff*cimag(center)),

        .x_coeff_coaxis = y_coeff,
        .y_coeff_coaxis = -x_coeff,
        .indep_coeff_coaxis = x_coeff*cimag(center) - y_coeff*creal(center),

        .color = color,

        .width = width,
        .length = length
    };
    imprint_ext(canvas, min, max, &_do_imprint_line_segment, &data);
}


typedef struct {
    complex_f f;
    const void *arg;
} imprint_function_as_hsv_data;

void _do_imprint_function_as_hsv(
        rgba_pixel *out, double complex z, const void *arg) {
    imprint_function_as_hsv_data data = 
        *((const imprint_function_as_hsv_data *) arg);
    float h, s, v;
    complex_to_color(data.f(z, data.arg), &h, &v);
    s = 1.0f;
    hsv_to_rgb(h, s, v, out);
    out->a = 255;
}


void imprint_function_as_hsv(
        rgba_image *canvas,
        double complex min, double complex max,
        complex_f f, const void *arg) {
    imprint_function_as_hsv_data data = {
        .f = f,
        .arg = arg
    };
    imprint_ext(canvas, min, max, &_do_imprint_function_as_hsv, &data);
}
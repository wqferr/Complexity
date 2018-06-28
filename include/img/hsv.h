#ifndef HSV_H
#define HSV_H 1

#include "img/rgbaimg.h"

void hsv_from_rgb(
        rgba_pixel pixel,
        float *h,
        float *s,
        float *v);
void hsv_to_rgb(
        float h,
        float s,
        float v,
        rgba_pixel *out);

#endif
#include "img/hsv.h"

#include <math.h>

#include <stdio.h>

void hsv_from_rgb(
        rgba_pixel pixel,
        float *h,
        float *s,
        float *v) {
    float r, g, b;
    float cmax;
    float cmin;
    float delta;
    char maxcomponent = 'r';

    r = (float) pixel.r / 255;
    g = (float) pixel.g / 255;
    b = (float) pixel.b / 255;

    cmax = r;
    cmin = r;

    if (g > cmax) {
        cmax = g;
        maxcomponent = 'g';
    }
    if (b > cmax) {
        cmax = b;
        maxcomponent = 'b';
    }

    if (g < cmin) {
        cmin = g;
    }
    if (b < cmin) {
        cmin = b;
    }

    delta = cmax - cmin;

    if (cmax == 0.0f) {
        *s = 0.0f;
        *h = 0.0f;
    } else {
        *s = delta / cmax;
        switch (maxcomponent) {
            case 'r':
                *h = fmodf(6.0f + (g - b) / delta, 6.0f);
                break;
            case 'g':
                *h = ((b - r) / delta) + 2;
                break;
            case 'b':
                *h = ((r - g) / delta) + 4;
                break;
        }
        *h *= 60.0f;
    }
    
    *v = cmax;
}


void hsv_to_rgb(
        float h,
        float s,
        float v,
        rgba_pixel *out) {
    float c = v * s;
    float x;
    float m = v - c;
    h /= 60.0f;
    x = c * (1 - fabsf(fmodf(h, 2.0f) - 1));
    if (h <= 1.0f) {
        out->r = 255 * c;
        out->g = 255 * x;
        out->b = 0;
    } else if (h <= 2.0f) {
        out->r = 255 * x;
        out->g = 255 * c;
        out->b = 0;
    } else if (h <= 3.0f) {
        out->r = 0;
        out->g = 255 * c;
        out->b = 255 * x;
    } else if (h <= 4.0f) {
        out->r = 0;
        out->g = 255 * x;
        out->b = 255 * c;
    } else if (h <= 5.0f) {
        out->r = 255 * x;
        out->g = 0;
        out->b = 255 * c;
    } else {
        out->r = 255 * c;
        out->g = 0;
        out->b = 255 * x;
    }

    out->r += 255 * m;
    out->g += 255 * m;
    out->b += 255 * m;
}
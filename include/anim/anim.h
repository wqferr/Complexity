#ifndef ANIM_H
#define ANIM_H

#include "img/rgbaimg.h"

typedef rgba_image *(*create_frame_f)(const void *arg, double progress);
typedef double (*time_smoothing_f)(double progress);

void animate(
    create_frame_f create_frame, const void *arg, time_smoothing_f smooth,
    size_t n_frames, const char *out_dir);

double anim_time_linear(double progress);
double anim_time_sigmoid(double progress);
double anim_time_smoothstep(double progress);
double anim_time_smootherstep(double progress);
double anim_time_smootheststep(double progress);

#endif
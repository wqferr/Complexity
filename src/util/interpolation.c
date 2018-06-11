#include "util/interpolation.h"


double lerp(double x0, double x1, double t) {
    return (1-t)*x0 + t*x1;
}

double complex clerp(double complex z0, double complex z1, double t) {
    return (1-t)*z0 + t*z1;
}
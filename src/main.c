#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"

#include "core/transform.h"

double complex f(double complex z) {
	return cexp(z);
}

int main(int argc, char const *argv[]) {
	rgba_image *in_img;
	rgba_image *out_img;

	png_load_from_file(&in_img, "img/5_big.png");
	out_img = warp_ext(
		in_img, &f,
		0+0j, 1+M_PI/2 * I,
		0+0j, 2+2j,
		0, 0);
	png_save_to_file(out_img, "img/out.png");
	rgbaimg_destroy(in_img);
	rgbaimg_destroy(out_img);

	return 0;
}
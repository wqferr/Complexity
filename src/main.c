#include <stdio.h>
#include <stdint.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"

#include "core/transform.h"

double complex f(double complex z) {
	return (z*z + 1) / 2;
}

int main(int argc, char const *argv[]) {
	rgba_image *in_img;
	rgba_image *out_img;

	png_load_from_file(&in_img, "img/5.png");
	out_img = warp(in_img, &f);
	png_save_to_file(out_img, "img/out.png");
	rgbaimg_destroy(in_img);
	rgbaimg_destroy(out_img);

	return 0;
}
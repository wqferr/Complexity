#include <stdio.h>
#include <stdint.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"

void print_pixel(rgba_image *img, size_t x, size_t y) {
	rgba_pixel pixel;

	rgbaimg_get_pixel(img, x, y, &pixel);
	printf(
		"(%hhu, %hhu, %hhu, %hhu) ",
		pixel.r, pixel.g, pixel.b, pixel.a);
}

int main(int argc, char const *argv[]) {
	rgba_image *img;
	int x;
	size_t w, h;
	rgba_pixel pixel = {0, 0, 0, 255};

	png_load_from_file(&img, "img/0.png");
	rgbaimg_get_dimensions(img, &w, &h);

	for (x = 0; x < w; x++) {
		rgbaimg_set_pixel(img, x, 0, pixel);
	}

	png_save_to_file(img, "img/out.png");
	rgbaimg_destroy(img);
	return 0;
}
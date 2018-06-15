#include "img/rgbaimg.h"

#include <stdlib.h>

#define MAT_TO_IDX(img, x, y) ((x) + (y*img->width))
#define MAT_IDX_OOB(img, idx) (((idx) >= (img)->width * (img)->height) || (idx) < 0)

/**
 * @brief      A matrix of rgba_pixels.
 */
struct rgba_image {
	/* Pixel matrix in vector form */  
	rgba_pixel *pixels;

	/* Image width */
	size_t width;

	/* Image height */
	size_t height;
};

rgba_image *rgbaimg_create(size_t w, size_t h) {
	size_t i, n;
	rgba_image *img = malloc(sizeof(*img));
	img->pixels = calloc(w * h, sizeof(*img->pixels));
	img->width = w;
	img->height = h;

	n = w*h;
	for (i = 0; i < n; i++) {
		img->pixels[i].a = UINT8_MAX;
	}

	return img;
}

void rgbaimg_destroy(rgba_image *img) {
	free(img->pixels);
	free(img);
}

inline void rgbaimg_get_dimensions(const rgba_image *img, size_t *width, size_t *height) {
	*width = img->width;
	*height = img->height;
}

inline int rgbaimg_set_pixel(
	rgba_image *img,
	size_t x, size_t y,
	rgba_pixel pixel) {

	int idx = MAT_TO_IDX(img, x, y);
	if (MAT_IDX_OOB(img, idx)) {
		return 1;
	}

	img->pixels[idx] = pixel;
	return 0;
}

inline int rgbaimg_set_rgba(
	rgba_image *img,
	size_t x, size_t y,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a) {

	rgba_pixel p = {r, g, b, a};
	return rgbaimg_set_pixel(img, x, y, p);
}

inline int rgbaimg_get_pixel(
	const rgba_image *img,
	size_t x, size_t y,
	rgba_pixel *pixel) {

	int idx = MAT_TO_IDX(img, x, y);
	if (MAT_IDX_OOB(img, idx)) {
		return 1;
	}

	*pixel = img->pixels[idx];
	return 0;
}
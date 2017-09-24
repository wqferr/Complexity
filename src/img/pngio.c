#include "img/pngio.h"

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <png.h>

#define PNG_HEADER_SIZE 8
#define PNG_BITDEPTH 8

int png_save_to_file(const rgba_image *img, const char *filename) {
	int i, j;
	size_t width;
	size_t height;
	rgba_pixel pixel;

	png_structp png_ptr;
	png_infop info_ptr;
	FILE *imfile;
	png_byte **rows;

	imfile = fopen(filename, "wb");
	if (imfile == NULL) {
		return 1; /* Could not open file */
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		return 2; /* Could not create write structure */
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		return 3; /* Could not create info structure */
	}

	rgbaimg_get_dimensions(img, &width, &height);

	png_init_io(png_ptr, imfile);
	png_set_IHDR(
		png_ptr, info_ptr,
		width, height,
		PNG_BITDEPTH,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	rows = malloc(height * sizeof(*rows));
	for (i = 0; i < height; i++) {
		rows[i] = malloc(4 * width * sizeof(**rows)); /* 4 channels per pixel */
		for (j = 0; j < width; j++) {
			rgbaimg_get_pixel(img, j, i, &pixel);

			rows[i][4*j] = pixel.r;
			rows[i][4*j+1] = pixel.g;
			rows[i][4*j+2] = pixel.b;
			rows[i][4*j+3] = pixel.a;
		}
	}

	png_write_image(png_ptr, rows);

	for (i = 0; i < height; i++)
		free(rows[i]);
	free(rows);

	fclose(imfile);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	return 0;
}

int png_load_from_file(rgba_image **img, const char *filename) {
	png_byte bytes_per_pixel;
	int i, j, k;
	size_t width;
	size_t height;
	rgba_pixel pixel;

	uint8_t header[PNG_HEADER_SIZE];
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	FILE *imfile;

	bool has_alpha;

	png_byte **rows;

	imfile = fopen(filename, "rb");
	if (imfile == NULL) {
		return 1; /* File not found */
	}

	fread(header, 1, PNG_HEADER_SIZE, imfile);
	if (png_sig_cmp(header, 0, PNG_HEADER_SIZE) != 0) {
		return 2; /* Not a PNG file */
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		return 3; /* Could not create read structure */
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		return 4; /* Could not create info structure */
	}
	end_info_ptr = png_create_info_struct(png_ptr);
	if (end_info_ptr == NULL) {
		return 5; /* IDK, could happen I guess */
	}

	png_init_io(png_ptr, imfile);
	png_set_sig_bytes(png_ptr, PNG_HEADER_SIZE); /* Flag signature as already read */
	png_read_info(png_ptr, info_ptr);

	if (png_get_bit_depth(png_ptr, info_ptr) < PNG_BITDEPTH) {
		png_set_packing(png_ptr); /* Pack to 8 bits per channel */
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr); /* Add alpha channel if transparency exists */
	}

	/* Convert to RGB or RGBA */
	switch (png_get_color_type(png_ptr, info_ptr)) {
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			png_set_gray_to_rgb(png_ptr);
			break;

		case PNG_COLOR_TYPE_PALETTE:
			png_set_expand(png_ptr);
			break;

		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGBA:
			break;

		default:
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);
			return 6;
	}

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	bytes_per_pixel = png_get_rowbytes(png_ptr, info_ptr) / width;
	has_alpha = (bytes_per_pixel == 4);

	png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	rows = malloc(height * sizeof(*rows));
	for (i = 0; i < height; i++) {
		rows[i] = malloc(png_get_rowbytes(png_ptr, info_ptr));
	}

	png_read_image(png_ptr, rows);
	fclose(imfile);

	*img = rgbaimg_create(width, height);
	for (i = 0; i < height; i++) {
		for (j = 0, k = 0; j < width; j++, k += bytes_per_pixel) {
			pixel.r = rows[i][k];
			pixel.g = rows[i][k+1];
			pixel.b = rows[i][k+2];
			if (has_alpha) {
				pixel.a = rows[i][k+3];
			} else {
				pixel.a = UINT8_MAX;
			}
			rgbaimg_set_pixel(*img, j, i, pixel);
		}
	}

	for (i = 0; i < height; i++) {
		free(rows[i]);
	}
	free(rows);

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);
	return 0;
}
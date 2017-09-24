#ifndef IMAGE_H
#define IMAGE_H 1

#include <stddef.h>
#include <stdint.h>

typedef struct rgba_pixel rgba_pixel;

typedef struct rgba_image rgba_image;


/**
 * @brief      An RGBA pixel.
 */
struct rgba_pixel {
	/* Red component */
	uint8_t r;

	/* Green component */
	uint8_t g;

	/* Blue component */
	uint8_t b;

	/* Alpha component */
	uint8_t a;
};


/**
 * @brief      Creates a new RGB pixel matrix.
 *
 * @param[in]  width   The width of the matrix
 * @param[in]  height  The height of the matrix
 *
 * @return     A pointer to png_image structure created.
 */
rgba_image *rgbaimg_create(size_t width, size_t height);

/**
 * @brief      Destroys a given RGBA image.
 *
 * @param[in]  img   The image
 */
void rgbaimg_destroy(rgba_image *img);

/**
 * @brief      Gets the dimensions of the image.
 *
 * @param[in]  img     The image
 * @param[out] width   The width
 * @param[out] height  The height
 */
void rgbaimg_get_dimensions(const rgba_image *img, size_t *width, size_t *height);

/**
 * @brief      Sets a pixel of the image.
 *
 * @param[in]  img    The image
 * @param[in]  x      The X coordinate of the pixel to be set
 * @param[in]  y      The Y coordinate of the pixel to be set
 * @param[in]  pixel  The new value of the pixel
 *
 * @return     Zero if the operation was successfull. The error code generated
 *             otherwise.
 */
int rgbaimg_set_pixel(
	rgba_image *img,
	size_t x, size_t y,
	rgba_pixel pixel);

/**
 * @brief      Sets a pixel of the image given its RGB components.
 *
 * @param[in]  img   The image
 * @param[in]  x     The X coordinate of the pixel to be set
 * @param[in]  y     The X coordinate of the pixel to be set
 * @param[in]  r     The new R component of the pixel
 * @param[in]  g     The new G component of the pixel
 * @param[in]  b     The new B component of the pixel
 * @param[in]  a     The new A component of the pixel
 *
 * @return     Zero if the operation was successfull. The error code generated
 *             otherwise.
 */
int rgbaimg_set_rgb(
	rgba_image *img,
	size_t x, size_t y,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * @brief      Gets a pixel of the image given its coordinates.
 *
 * @param[in]  img    The image
 * @param[in]  x      The X coordinate of the pixel to be retreived
 * @param[in]  y      The Y coordinate of the pixel to be retreived
 * @param[out] pixel  The address into which the pixel data will be stored
 *
 * @return     Zero if the operation was successfull. The error code generated
 *             otherwise.
 */
int rgbaimg_get_pixel(
	const rgba_image *img,
	size_t x, size_t y,
	rgba_pixel *pixel);

#endif
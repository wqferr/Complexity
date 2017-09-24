#ifndef IMG_IO_H
#define IMG_IO_H 1

#include "img/rgbaimg.h"

/**
 * @brief      Saves an RGB image to a file according to PNG format.
 *
 * @param[in]  img       The image
 * @param[in]  filename  The filename
 *
 * @return     Zero if the operation was successfull. The error code generated
 *             otherwise.
 */
int png_save_to_file(const rgba_image *img, const char *filename);

/**
 * @brief      Loads an RGB image from a PNG file.
 *
 * @param[out] img       The reference to the variable image
 * @param[in]  filename  The filename
 *
 * @return     Zero if the operation was successfull. The error code generated
 *             otherwise.
 */
int png_load_from_file(rgba_image **img, const char *filename);

#endif
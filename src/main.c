#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <omp.h>
#include <math.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"

#include "core/cfunction.h"

#include "anim/anim.h"

#include "util/interpolation.h"
#include "util/misc.h"
#include "util/imprint.h"

#define IMG_PATH_PREFIX "img/"
#define IMG_PATH_SUFFIX ".png"

#define FRAMES_OUTPUT_DIR "img/out/"
#define OUTPUT_FPS 60


double complex f(double complex z, const void *arg) {
	double t = *((const double *) arg);
	return cpow(z, clerp(1, 3, t));
}

double complex heart_f(double complex z, const void *arg) {
	double t = *((const double *) arg);
	return cpow(z, clerp(5, 1, t));
}

void clean_dir(const char *path);

void read_input(int argc, char *const argv[], rgba_image **img, size_t *n_frames);
rgba_image *create_frame(double progress, const void *arg);
void imprint_heart_thing(rgba_pixel *color, double complex z, const void *arg);

int create_imprint(int argc, char *const argv[]);
int create_anim(int argc, char *const argv[]);

int main(int argc, char *const argv[]) {
	return create_imprint(argc, argv);
}

int create_anim(int argc, char *const argv[]) {
	rgba_image *in_img;
	size_t n_frames;
	size_t inwidth, inheight;

	read_input(argc, argv, &in_img, &n_frames);
	rgbaimg_get_dimensions(in_img, &inwidth, &inheight);
	omp_set_nested(true);
	
	clean_dir(FRAMES_OUTPUT_DIR);

	animate(
		&create_frame, in_img,
		&anim_time_smootheststep, n_frames,
		"img/out/");

	rgbaimg_destroy(in_img);
	return 0;
}

int create_imprint(int argc, char *const argv[]) {
	rgba_image *out;
	rgba_pixel pink = { .r = 255, .g = 120, .b = 120, .a = 255 };

	out = rgbaimg_create(500, 500);
	imprint_ext(
		out,
		(-1-1i), (+1+1i),
		&imprint_heart_thing, &pink);
	imprint_line_segment(
		out,
		(-1-1i), (+1+1i),
		pink,
		1, 0, 0+0.62i,
		0.025, 0.65);
	png_save_to_file(out, "img/imprint.png");
	rgbaimg_destroy(out);

	return 0;
}


void imprint_heart_thing(rgba_pixel *color, double complex z, const void *arg) {
	double x = creal(z);
	double y = cimag(z) + 0.75;
	double abs_x = fabs(x);
	rgba_pixel col = *((const rgba_pixel *) arg);

	if (fabs(abs_x*log(abs_x) - (abs_x - y)*log(y)) < 0.05) {
		*color = col;
	}
}



void imprint_name(rgba_image *canvas, rgba_pixel color) {
	double complex d_center = -0.475-0.05i;
	double d_min_radius = 0.175;
	double d_max_radius = 0.25;
	double d_rot = 0.3;
	double sec_d_rot = 1/cos(d_rot);
	double csc_d_rot = 1/sin(d_rot);
	double line_width = (d_max_radius-d_min_radius);

	imprint_circle(
		canvas,
		(-1-1i), (+1+1i),
		color,
		d_center,
		d_min_radius, d_max_radius,
		-M_PI/2+d_rot, M_PI/2+d_rot);
	imprint_line_segment(
		canvas, (-1-1i), (+1+1i),
		color,
		-csc_d_rot, -sec_d_rot,
		d_center,
		line_width, 2*d_max_radius-0.01);
	imprint_line_segment(
		canvas,
		(-1-1i), (+1+1i),
		color,
		4, -1,
		(-0.0625+0i), line_width, 0.5);
	imprint_line_segment(
		canvas,
		(-1-1i), (+1+1i),
		color,
		4, 1,
		(0.0625+0i), line_width, 0.5);
	imprint_line_segment(
		canvas,
		(-1-1i), (+1+1i),
		color,
		0, 1,
		(0+0.125i), 2*line_width/3, 0.25);
	imprint_line_segment(
		canvas,
		(-1-1i), (+1+1i),
		color,
		1, 0.4,
		(0.45+0.15i), line_width, 0.25);
	imprint_line_segment(
		canvas,
		(-1-1i), (+1+1i),
		color,
		1, -0.8,
		(0.5-0.1i), line_width, 0.5);
}


void read_input(int argc, char *const argv[], rgba_image **img, size_t *n_frames) {
	char *imgid;
	char *path;
	int c;
	float out_duration;

	while ((c = getopt(argc, argv, "i:t:")) != -1) {
		switch (c) {
			case 't':
				sscanf(optarg, "%f", &out_duration);
				*n_frames = out_duration * OUTPUT_FPS;
				break;
			case 'i':
				imgid = strdup(optarg);
				break;
		}
	}

	sprintf_alloc(&path, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
	png_load_from_file(img, path);

	free(imgid);
	free(path);
}

rgba_image *create_frame(double progress, const void *arg) {
	size_t w, h;
	const rgba_image *input = (const rgba_image *) arg;
	rgbaimg_get_dimensions(input, &w, &h);
	return warp_ext(
		input, &f, &progress,
		(-1-1i), (+1+1i),
		(-1-1i), (+1+1i),
		w, h);
}

rgba_image *create_heart_frame(double progress, const void *arg) {
	size_t w, h;
	const rgba_image *input = (const rgba_image *) arg;
	rgbaimg_get_dimensions(input, &w, &h);
	return warp_ext(
		input, &heart_f, &progress,
		(-1-1i), (+1+1i),
		(-1-1i), (+1+1i),
		w, h);
}

void clean_dir(const char *path) {
	char *cmd;
	sprintf_alloc(&cmd, "rm -f %s/* --preserve-root", path);
	system(cmd);
	free(cmd);
}
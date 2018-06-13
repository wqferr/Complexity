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

#define IMG_PATH_PREFIX "img/"
#define IMG_PATH_SUFFIX ".png"

#define FRAMES_OUTPUT_DIR "img/out/"


double complex f(double complex z, const void *arg) {
	double t = *((const double *) arg);
	return cpow(z, clerp(1, 0.25, t));
}

void clean_dir(const char *path);

rgba_image *read_input_img(int argc, char *const argv[]);
rgba_image *create_frame(const void *arg, double progress);

int main(int argc, char *const argv[]) {
	rgba_image *in_img;
	size_t n_frames;
	size_t inwidth, inheight;

	in_img = read_input_img(argc, argv);
	rgbaimg_get_dimensions(in_img, &inwidth, &inheight);
	if (getopt(argc, argv, "n:") != -1) {
		n_frames = atoi(optarg);
	}
/*
	outwidth = inwidth;
	outheight = inheight;
	*/

	omp_set_nested(true);
	
	clean_dir(FRAMES_OUTPUT_DIR);

	animate(
		&create_frame, in_img,
		&anim_time_smootheststep, n_frames,
		"img/out/");

	
/*
	#pragma omp parallel for\
		num_threads(4)\
		private(out_img, interp_time)\
		shared(in_img)
	for (cur_frame = 0; cur_frame < n_frames; cur_frame++) {
		fprintf(stderr, "Processing frame %zu\n", cur_frame);
		interp_time = time(cur_frame, n_frames);
		out_img = warp_ext(
			in_img, &f, &interp_time,
			(-1-1i), (1+1i),
			(-1-1i), (1+1i),
			outwidth, outheight);
		save_frame(out_img, cur_frame);
		rgbaimg_destroy(out_img);
	}
*/
	rgbaimg_destroy(in_img);
	return 0;
}


rgba_image *read_input_img(int argc, char *const argv[]) {
	char *imgid;
	char *path;
	/* size_t pathsize; */
	rgba_image *input;

	if (getopt(argc, argv, "i:") != -1) {
		imgid = strdup(optarg);
	}

	sprintf_alloc(&path, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
/*
	pathsize = snprintf(NULL, 0, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
	path = malloc((pathsize + 1) * sizeof(*path));
	sprintf(path, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
	*/
	png_load_from_file(&input, path);

	free(imgid);
	free(path);
	return input;
}

rgba_image *create_frame(const void *arg, double progress) {
	size_t w, h;
	const rgba_image *input = (const rgba_image *) arg;
	rgbaimg_get_dimensions(input, &w, &h);
	return warp_ext(
		input, &f, &progress,
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

/*
void save_frame(rgba_image *frame, size_t n) {
	char *path;
	size_t pathsize;

	pathsize = snprintf(NULL, 0, "%sout/%03zu%s", IMG_PATH_PREFIX, n, IMG_PATH_SUFFIX);
	path = malloc((pathsize + 1) * sizeof(*path));
	sprintf(path, "%sout/%03zu%s", IMG_PATH_PREFIX, n, IMG_PATH_SUFFIX);
	png_save_to_file(frame, path);
	free(path);
}


#define STEEPNESS 1
#define SIGMOID_START -10
#define SIGMOID_END 15
float time(size_t frame, size_t n_frames) {
	float t;
	if (frame == 0) {
		return 0;
	}
	t = lerp(SIGMOID_START, SIGMOID_END, (double) frame / n_frames);
	return 1.0f / (1 + exp(STEEPNESS * (-t)));
}*/
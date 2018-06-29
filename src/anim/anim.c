#include "anim/anim.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"
#include "util/misc.h"

void _save_frame(
        const rgba_image *frame,
        size_t n, size_t n_frames,
        const char *out_dir,
        size_t *len_n_frames);

void animate(
        create_frame_f create_frame, const void *arg, time_smoothing_f smooth,
        size_t n_frames, const char *out_dir) {
    int cur_frame;
    double progress;
    rgba_image *output;
    char *cmd;
    size_t len_n_frames;

    if (smooth == NULL) {
        smooth = &anim_time_linear;
    }

    #pragma omp parallel for\
            num_threads(4)\
            private(output, progress)\
            shared(arg, create_frame, smooth, n_frames, out_dir)
    for (cur_frame = 0; cur_frame < n_frames; cur_frame++) {
        #pragma omp critical
        {
            fprintf(stderr, "Creating frame %d\n", cur_frame);
        }
        progress = smooth((double) cur_frame / n_frames);
        output = create_frame(progress, arg);
        _save_frame(output, cur_frame, n_frames, out_dir, &len_n_frames);
        rgbaimg_destroy(output);
    }
    fprintf(stderr, "Done creating frames\n");
    
    sprintf_alloc(&cmd,
        "ffmpeg -y -framerate 60 -i img/out/%%0%zud.png "
		"-c:v libx264 -pix_fmt yuv420p img/out.mp4",
        len_n_frames);
    system(cmd);
    free(cmd);
}


void _save_frame(
        const rgba_image *frame,
        size_t n, size_t n_frames,
        const char *out_dir,
        size_t *len_n_frames) {
	char *path;
    char *pattern;
    *len_n_frames = snprintf(NULL, 0, "%zu", n_frames);

    sprintf_alloc(&pattern, "%%s/%%0%zuzu.png", *len_n_frames);
    sprintf_alloc(&path, pattern, out_dir, n);
	png_save_to_file(frame, path);

	free(path);
    free(pattern);
}


double anim_time_linear(double t) {
    return t;
}


double anim_time_sigmoid(double t) {
    return 1.0 / (1 + exp(-t));
}


double anim_time_smoothstep(double t) {
    if (t < 0) {
        return 0;
    } else if (t > 1) {
        return 1;
    }
    return t*t * (3 - 2*t);
}


double anim_time_smootherstep(double t) {
    if (t < 0) {
        return 0;
    } else if (t > 1) {
        return 1;
    }
    return t*t*t * (t * (6*t - 15) + 10);
}


double anim_time_smootheststep(double t) {
    double t3 = t*t*t;
    double t4 = t * t3;
    if (t < 0) {
        return 0;
    } else if (t > 1) {
        return 1;
    }
    return -20 * t4*t3 + 70 * t3*t3 - 84 * t4*t + 35 * t4;
}
#include "bitmap.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

// Define a structure for thread arguments
typedef struct {
    struct bitmap *bm;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int max;
    int start_line;
    int num_lines;
} thread_args;

int iterations_at_point(double x, double y, int max);
void *compute_image_thread(void *args);

// Compute the number of iterations at point x, y in the Mandelbrot space
int iterations_at_point(double x, double y, int max) {
    double x0 = x;
    double y0 = y;
    int iter = 0;

    while ((x * x + y * y <= 4) && iter < max) {
        double xt = x * x - y * y + x0;
        double yt = 2 * x * y + y0;

        x = xt;
        y = yt;

        iter++;
    }

    return iter;
}

// Thread function for computing image
void *compute_image_thread(void *args) {
    thread_args *targs = (thread_args *)args;
    int width = bitmap_width(targs->bm);
    int height = targs->start_line + targs->num_lines;

    for (int j = targs->start_line; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double x = targs->xmin + i * (targs->xmax - targs->xmin) / width;
            double y = targs->ymin + j * (targs->ymax - targs->ymin) / height;

            int iters = iterations_at_point(x, y, targs->max);
            bitmap_set(targs->bm, i, j, iters);
        }
    }

    return NULL;
}

// Main program function
int main(int argc, char *argv[]) {
    char c;

    // Default values for the Mandelbrot set
    double xcenter = -0.5;
    double ycenter = -0.5;
    double scale = 4;
    int image_width = 500;
    int image_height = 500;
    int max = 1000;
    char *outfile = "mandel.bmp";
    int num_threads = 1;

    // Parse command line options
    while ((c = getopt(argc, argv, "x:y:s:W:H:m:o:n:h")) != -1) {
        switch (c) {
            case 'x':
                xcenter = atof(optarg);
                break;
            case 'y':
                ycenter = atof(optarg);
                break;
            case 's':
                scale = atof(optarg);
                break;
            case 'W':
                image_width = atoi(optarg);
                break;
            case 'H':
                image_height = atoi(optarg);
                break;
            case 'm':
                max = atoi(optarg);
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 'h':
                printf("usage: %s -x center_x -y center_y -s scale -W width -H height -m max -o outfile.bmp -n threads\n", argv[0]);
                exit(0);
                break;
        }
    }

    // Display the configuration of the image
    printf("Mandelbrot set configuration:\n");
    printf(" Center: (%lf, %lf)\n", xcenter, ycenter);
    printf(" Scale: %lf\n", scale);
    printf(" Max iterations: %d\n", max);
    printf(" Image size: %dx%d\n", image_width, image_height);
    printf(" Output file: %s\n", outfile);
    printf(" Threads: %d\n", num_threads);

    // Create a bitmap to hold the result
    struct bitmap *bm = bitmap_create(image_width, image_height);

    // Allocate thread structures
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_args *t_args = malloc(num_threads * sizeof(thread_args));

    // Initialize and create threads
    for (int i = 0; i < num_threads; i++) {
        t_args[i].bm = bm;
        t_args[i].xmin = xcenter - scale;
        t_args[i].xmax = xcenter + scale;
        t_args[i].ymin = ycenter - scale;
        t_args[i].ymax = ycenter + scale;
        t_args[i].max = max;
        t_args[i].start_line = (image_height / num_threads) * i;
        t_args[i].num_lines = (i == num_threads - 1) ? (image_height - t_args[i].start_line) : (image_height / num_threads);

        pthread_create(&threads[i], NULL, compute_image_thread, (void *)&t_args[i]);
    }

    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Save the image
    bitmap_save(bm, outfile);

    // Clean up
    free(threads);
    free(t_args);
    bitmap_delete(bm);

    return 0;
}

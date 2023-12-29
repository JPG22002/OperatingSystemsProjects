#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number of processes>\n", argv[0]);
        exit(1);
    }

    int num_processes = atoi(argv[1]);
    int active_processes = 0;
    double scale = 2.0;
    const double target_scale = 0.0001;
    const int frames = 50;

    for (int i = 0; i < frames; i++) {
        if (active_processes >= num_processes) {
            wait(NULL);
            active_processes--;
        }

        if (fork() == 0) {
            char filename[256];
            sprintf(filename, "mandel%d.bmp", i + 1);
            char scale_str[256];
            sprintf(scale_str, "%lf", scale);

            char *args[] = {"./mandel", "-x", "-0.5", "-y", "-0.5", "-s", scale_str, "-m", "1000", "-o", filename, NULL};
            execvp(args[0], args);
            fprintf(stderr, "Failed to execvp the mandel program.\n");
            exit(1);
        }

        active_processes++;
        scale *= exp(log(target_scale / scale) / (frames - 1));
    }

    while (active_processes > 0) {
        wait(NULL);
        active_processes--;
    }

    return 0;
}

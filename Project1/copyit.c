#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define BUF_SIZE 4096

// Function to display "still copying" message upon SIGALRM signal reception
static void DisplayMessage(int s) {
    printf("copyit: still copying...\n");
    fflush(stdout);
    alarm(1);
}

int main(int argc, char *argv[]) {
    // Check for user mistake in command-line arguments
    if (argc != 3) {
        fprintf(stderr, "copyit: Incorrect number of arguments!\n");
        fprintf(stderr, "usage: copyit <sourcefile> <targetfile>\n");
        return 1;
    }

    // Connect SIGALRM to display_message function
    signal(SIGALRM, DisplayMessage);
    alarm(1);  // Set the periodic message for every second

    int fd, fd2;

    // Open source file or exit with an error
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "copyit: Couldn't open file %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    // Create target file or exit with an error
    fd2 = creat(argv[2], 0644);
    if (fd2 < 0) {
        fprintf(stderr, "copyit: Couldn't create file %s: %s\n", argv[2], strerror(errno));
        close(fd);
        return 1;
    }

    char buffer[BUF_SIZE];
    ssize_t bytesRead, bytesWritten, totalBytes = 0;

    // Loop over file reading and writing
    while (1) {
        // Read data from source file
        bytesRead = read(fd, buffer, BUF_SIZE);

        // Retry read if interrupted by a signal
        while (bytesRead < 0 && errno == EINTR) {
            bytesRead = read(fd, buffer, BUF_SIZE);
        }

        // Exit with error if reading fails
        if (bytesRead < 0) {
            fprintf(stderr, "copyit: Error reading from %s: %s\n", argv[1], strerror(errno));
            close(fd);
            close(fd2);
            return 1;
        }

        // Break loop if no more data to read
        if (bytesRead == 0) {
            break;
        }

        // Write data to target file
        bytesWritten = write(fd2, buffer, bytesRead);

        // Retry write if interrupted by a signal
        while (bytesWritten < bytesRead && errno == EINTR) {
            bytesWritten = write(fd2, buffer + bytesWritten, bytesRead - bytesWritten);
        }

        // Exit with error if not all data was written
        if (bytesWritten < bytesRead) {
            fprintf(stderr, "copyit: Error writing to %s: %s\n", argv[2], strerror(errno));
            close(fd);
            close(fd2);
            return 1;
        }

        // Accumulate total bytes written
        totalBytes += bytesWritten;
    }

    // Close both files after copying
    close(fd);
    close(fd2);

    // Print success message after completion
    printf("copyit: Copied %ld bytes from file %s to %s.\n", totalBytes, argv[1], argv[2]);
    fflush(stdout);

    return 0;
}

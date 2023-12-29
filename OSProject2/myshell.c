#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT 4096
#define MAX_WORDS 100

// Function declarations
void runCommand(char *words[]);
void startCommand(char *words[]);
void waitCommand();
void killCommand(char *words[]);
void stopCommand(char *words[]);
void continueCommand(char *words[]);

int main() {
    char input[MAX_INPUT];  // buffer for user input
    char *words[MAX_WORDS]; // array to store the parsed words of the command

    // infinite loop for command reading
    while (1) {
        printf("myshell> ");
        fflush(stdout);  // make sure the prompt is displayed quickly

        // read user input
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            // exit the shell at the end of the file
            break;
        }

        // tokenize the input into words
        int nwords = 0;
        char *token = strtok(input, " \t\n");
        while (token != NULL && nwords < MAX_WORDS - 1) {
            words[nwords++] = token;
            token = strtok(NULL, " \t\n");
        }
        words[nwords] = NULL;  //  terminate the words array

        // execute the appropriate command based on the input
        if (nwords == 0) {
            // empty input, just reprint the prompt
            continue;
        } else if (strcmp(words[0], "exit") == 0 || strcmp(words[0], "quit") == 0) {
            // Exit  shell
            exit(0);
        } else if (strcmp(words[0], "run") == 0) {
            runCommand(words);
        } else if (strcmp(words[0], "start") == 0) {
            startCommand(words);
        } else if (strcmp(words[0], "wait") == 0) {
            waitCommand();
        } else if (strcmp(words[0], "kill") == 0) {
            killCommand(words);
        } else if (strcmp(words[0], "stop") == 0) {
            stopCommand(words);
        } else if (strcmp(words[0], "continue") == 0) {
            continueCommand(words);
        } else {
            // Unknown command
            printf("myshell: unknown command: %s\n", words[0]);
        }
    }
    return 0;
}

//  'run' command
void runCommand(char *words[]) {
    if (words[1] == NULL) {
        // missing program name
        printf("myshell: missing program name\n");
        return;
    }

    // make a child process
    pid_t pid = fork();
    if (pid == 0) {
        // child process
        execvp(words[1], &words[1]);
        // an error occurred
        printf("myshell: unable to run %s\n", words[1]);
        exit(1);
    } else if (pid < 0) {
        // error in the fork
        perror("myshell: error in fork");
        return;
    } else {
        // parent process waits for the child to finish
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("myshell: process %d exited normally with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("myshell: process %d exited abnormally with signal %d: %s\n", pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
    }
}

// 'start' command
void startCommand(char *words[]) {
    if (words[1] == NULL) {
        // missing program name
        printf("myshell: missing program name\n");
        return;
    }

    // make a child process
    pid_t pid = fork();
    if (pid == 0) {
        // child process
        execvp(words[1], &words[1]);
        // an error occurred
        printf("myshell: unable to start %s\n", words[1]);
        exit(1);
    } else if (pid < 0) {
        // fork error
        perror("myshell: error in fork");
    } else {
        // parent process, print the process ID and return immediately
        printf("myshell: process %d started\n", pid);
    }
}

// 'wait' command
void waitCommand() {
    int status;
    pid_t pid = wait(&status);

    if (pid > 0) {
        // print the right messages based on the exit status of the child process
        if (WIFEXITED(status)) {
            printf("myshell: process %d exited normally with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("myshell: process %d exited abnormally with signal %d: %s\n", pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
    } else {
        // no child processes
        printf("myshell: no processes left\n");
    }
}

// 'kill' command
void killCommand(char *words[]) {
    if (words[1] == NULL) {
        // process ID no there
        printf("myshell: missing process ID\n");
        return;
    }

    pid_t pid = atoi(words[1]);
    if (kill(pid, SIGKILL) == 0) {
        // Process killed 
        printf("myshell: process %d killed\n", pid);
    } else {
        // error in killing process
        perror("myshell: error in kill");
    }
}

// 'stop' command
void stopCommand(char *words[]) {
    if (words[1] == NULL) {
        // process ID not there
        printf("myshell: missing process ID\n");
        return;
    }

    pid_t pid = atoi(words[1]);
    if (kill(pid, SIGSTOP) == 0) {
        // Process stopped successfully
        printf("myshell: process %d stopped\n", pid);
    } else {
        // Error in stopping process
        perror("myshell: error in stop");
    }
}

// Implement the 'continue' command
void continueCommand(char *words[]) {
    if (words[1] == NULL) {
        // Missing process ID
        printf("myshell: missing process ID\n");
        return;
    }

    pid_t pid = atoi(words[1]);
    if (kill(pid, SIGCONT) == 0) {
        // process continued worked right YAYYY
        printf("myshell: process %d continued\n", pid);
    } else {
        // error continuing process BOOOO
        perror("myshell: error in continue");
    }
}
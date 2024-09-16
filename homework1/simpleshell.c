#include "simpleshell.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int parseInput(char *input, char splitWords[][500], int maxWords) {
    char *token = strtok(input, " \t\n");
    int count = 0;

    while (token != NULL && count <= maxWords) {
        strcpy(splitWords[count++], token);
        token = strtok(NULL, " \t\n");
    }

    return count;
}

int main() {
    char cwd[FILENAME_MAX];

    char input[500];
    char splitWords[500][500];
    int maxWords = 100;

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("teolson:%s$ ", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }

        fgets(input, sizeof(input), stdin);

        int wordCount = parseInput(input, splitWords, maxWords);
        if (wordCount == 0) continue;

        if (strcmp(splitWords[0], "exit") == 0) {
            break;
        } else if (strcmp(splitWords[0], "cd") == 0) {
            if (wordCount == 2) {
                changeDirectories(splitWords[1]);
            } else {
                printf("Path Not Formatted Correctly!\n");
            }
        } else {
            char *infile = NULL, *outfile = NULL;
            char *command[maxWords];
            int commandAmount = 0;

            for (int i = 0; i < wordCount; i++) {
                if (strcmp(splitWords[i], "<") == 0 && i + 1 < wordCount) {
                    infile = splitWords[i + 1];
                    i++;
                } else if (strcmp(splitWords[i], ">") == 0 &&
                           i + 1 < wordCount) {
                    outfile = splitWords[i + 1];
                    i++;
                } else {
                    command[commandAmount++] = splitWords[i];
                }
            }
            command[commandAmount] = NULL;
            executeCommand(command, infile, outfile);
        }
    }

    return 0;
}

void changeDirectories(const char *path) {
    if (chdir(path) == -1) {
        printf("chdir: Failed %s\n", strerror(errno));
    }
}

int executeCommand(char *const *enteredCommand, const char *infile,
                   const char *outfile) {
    pid_t p = fork();

    if (p < 0) {
        printf("fork: Failed %s\n", strerror(errno));
        return -1;
    } else if (p == 0) {  // child process
        if (infile != NULL) {
            int fd = open(infile, O_RDONLY, 0666);
            dup2(fd, STDIN_FILENO);  // STDIN_FILENO is 0
            close(fd);
        }

        if (outfile != NULL) {
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(fd, STDOUT_FILENO);  // STDOUT_FILENO is 1
            close(fd);
        }

        execvp(enteredCommand[0], enteredCommand);
        printf("execvp: Failed %s\n", strerror(errno));
        _Exit(1);
    } else {
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                printf("Child finished with error status: %d\n",
                       WEXITSTATUS(status));
            }
        }
        return 0;
    }

    return 0;
}
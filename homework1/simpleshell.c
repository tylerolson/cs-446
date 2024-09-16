#include "simpleshell.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
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
    while (1) {
        char cwd[FILENAME_MAX];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("teolson:%s$ ", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }

        char input[500];
        char splitWords[500][500];
        int validCommand = 0;

        fgets(input, sizeof(input), stdin);

        int wordsParsed = parseInput(input, splitWords, 100);

        if (strcmp(splitWords[0], "exit") == 0) {
            validCommand = 1;
            break;
        } else if (strcmp(splitWords[0], "cd") == 0) {
            validCommand = 1;

            if (wordsParsed == 2) {
                changeDirectories(splitWords[1]);
            } else {
                printf("Path Not Formatted Correctly!\n");
            }
        }

        if (validCommand == 0) {
            printf("command not found: %s\n", splitWords[0]);
        }
    }

    return 0;
}

void changeDirectories(const char *path) {
    if (chdir(path) == -1) {
        printf("chdir: Failed %s\n", errno, strerror(errno));
    }
}

int executeCommand(char *const *enteredCommand, const char *infile,
                   const char *outfile);
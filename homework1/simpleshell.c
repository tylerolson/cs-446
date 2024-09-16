#include "simpleshell.h"

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
        int maxWords = 100;

        fgets(input, sizeof(input), stdin);

        int wordsParsed = parseInput(input, splitWords, maxWords);

        printf("found %d\n", wordsParsed);
        if (strcmp(splitWords[0], "exit") == 0) {
            break;
        }
    }

    return 0;
}
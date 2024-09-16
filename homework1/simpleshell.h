#ifndef SIMPLESHELL_H
#define SIMPLESHELL_H

int parseInput(char *input, char splitWords[][500], int maxWords);
void changeDirectories(const char *path);
int executeCommand(char *const *enteredCommand, const char *infile, const char *outfile);

#endif
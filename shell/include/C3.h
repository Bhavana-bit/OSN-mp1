#ifndef C3_H
#define C3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Prints the shell-style prompt
void printprompt1();

// Parses and executes commands with input/output redirection
void execute_command_with_redirection(char *input);

#endif // C3_H

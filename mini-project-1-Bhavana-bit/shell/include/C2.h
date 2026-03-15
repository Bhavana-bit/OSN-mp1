#ifndef C2_H
#define C2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Prints shell prompt
void print_prompt();

// Parses input & executes command with optional input redirection
void execute_command_with_input(char *input);

#endif // C2_H

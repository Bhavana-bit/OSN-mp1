#ifndef C1_H
#define C1_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Prints the shell-style prompt (<user@host:cwd>)
void print_prompt();

// Executes a command string (tokenizes + fork + execvp)
void execute_command(char *input);

#endif // C1_H

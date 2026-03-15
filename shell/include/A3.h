#ifndef A3_H
#define A3_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <regex.h>
// Compile regex for shell command validation
int compile_syntax_regex(regex_t *regex);

// Validate a given command string against the regex
int validate_command(regex_t *regex, const char *input);

#endif

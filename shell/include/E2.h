#ifndef E2_H
#define E2_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#define MAX_JOBS 100
#define MAX_ARGS 100
#define MAX_CMD_LEN 256

#include "D2.h"

void inttostr(int num, char *str);
void update_job_states();
int compare_jobs(const void *a, const void *b);
void activities();

int is_valid_number(const char *s);
void ping_command(char *pid_str, char *sig_str);
void execute_e2_shell();

#endif

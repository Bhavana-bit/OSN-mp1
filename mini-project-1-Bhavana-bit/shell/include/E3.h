#ifndef E3_H
#define E3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#define MAX_JOBS 100
#define MAX_ARGS 100
#define MAX_CMD_LEN 1024

#include "D2.h"


extern pid_t fg_pid;

int is_number(const char *s);
void handle_eof();
void sigint_handler(int signum);
void sigtstp_handler(int signum);
void update_job_states();
int compare_jobs(const void *a, const void *b);
void activities_command();
void execute_e3_shell(); 

#endif

#ifndef E4_H
#define E4_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#define MAX_JOBS 100
#define MAX_ARGS 100
#define MAX_CMD_LEN 1024

#include "D2.h"

void inttostr(int num, char *str);
void update_job_states();
int compare_jobs(const void *a, const void *b);
void activities();
int is_number(const char *s);
int find_job_by_number(int job_num);
int get_most_recent_job();
void fg_command(char *job_num_str);
void bg_command(char *job_num_str);
void execute_e4_shell(); 

#endif

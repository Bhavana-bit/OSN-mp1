#ifndef E1_H
#define E1_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_JOBS 100
#define MAX_ARGS 100
#define MAX_CMD_LEN 256


#include "D2.h"


void inttostr(int num, char *str);
void update_job_states();
int compare_jobs(const void *a, const void *b);
void activities();
void execute_e1_shell();

#endif

#ifndef D2_H
#define D2_H

#include <unistd.h>

#define MAX_CMD_LEN 256
#define MAX_JOBS 100

// Struct definition
struct job {
    int job_number;
    pid_t pid;
    char command[MAX_CMD_LEN];
    char state[16];
};

// Declare global variables (no definition here)
extern struct job jobs[MAX_JOBS];
extern int job_count;
extern int job_number;

// Function declaration
void execute_d2_shell();

#endif

#ifndef B3_H
#define B3_H

#define MAX_LOG 15
#define LOG_FILE "log.txt"

void init_log();
void save_log();
void handle_log(char *input);

extern char log_arr[MAX_LOG][1024];
extern int log_count;

#endif

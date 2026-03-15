#include "B3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_log()
{
    FILE *fp = fopen(LOG_FILE, "r");
    if (!fp)
        return;
    while (fgets(log_arr[log_count], 1024, fp) && log_count < MAX_LOG)
    {
        size_t len = strlen(log_arr[log_count]);
        if (len > 0 && log_arr[log_count][len - 1] == '\n')
            log_arr[log_count][len - 1] = '\0';
        log_count++;
    }
    fclose(fp);
}

void save_log()
{
    FILE *f = fopen(LOG_FILE, "w");
    if (!f)
        return;
    int start = log_count > MAX_LOG ? log_count - MAX_LOG : 0;
    for (int i = start; i < log_count; i++)
        fprintf(f, "%s\n", log_arr[i % MAX_LOG]);
    fclose(f);
}

void handle_log(char *input)
{
    if (strcmp(input, "log") == 0)
    {
        int start = log_count > MAX_LOG ? log_count - MAX_LOG : 0;
        for (int i = start; i < log_count; i++)
            printf("%s\n", log_arr[i % MAX_LOG]);
    }
    else if (strcmp(input, "log purge") == 0)
    {
        log_count = 0;
        save_log();
    }
    else if (strncmp(input, "log execute ", 12) == 0)
    {
        char *idx_str = input + 12;
        int idx = atoi(idx_str);
        if (idx <= 0 || idx > log_count)
        {
            printf("Invalid index\n");
        }
        else
        {
            int cmd_index = log_count - idx;
            printf("%s\n", log_arr[cmd_index % MAX_LOG]);
            system(log_arr[cmd_index % MAX_LOG]);
        }
    }
    else
    {
        printf("Invalid log command\n");
    }
}

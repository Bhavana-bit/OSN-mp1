#include "E1.h"

void execute_e1_shell()
{
    while (1)
    {
        char input[2000];
        pid_t bg_pid;
        int status;

        // Check background jobs
        while ((bg_pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            for (int j = 0; j < job_count; j++)
            {
                if (jobs[j].pid == bg_pid)
                {
                    if (WIFEXITED(status))
                        printf("%s with pid %d exited normally\n", jobs[j].command, bg_pid);
                    else
                        printf("%s with pid %d exited abnormally\n", jobs[j].command, bg_pid);

                    for (int k = j; k < job_count - 1; k++)
                        jobs[k] = jobs[k + 1];

                    job_count--;
                    break;
                }
            }
        }

        // Build prompt
        char prompt[2000];
        build_prompt(prompt, sizeof(prompt));
        printf("%s\n", prompt);

        if (!fgets(input, sizeof(input), stdin))
            break;

        if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';
        if (strlen(input) == 0)
            continue;
        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "activities") == 0)
        {
            activities(); // call function from main.c
            continue;
        }

        // Check for background
        int len = strlen(input) - 1;
        while (len >= 0 && (input[len] == ' ' || input[len] == '\t'))
            len--;
        input[len + 1] = '\0';

        int background = 0;
        if (len >= 0 && input[len] == '&')
        {
            background = 1;
            input[len] = '\0';
        }

        // Parse arguments
        char *arg[MAX_ARGS];
        int argc = 0;
        char *token = strtok(input, " ");
        while (token != NULL)
        {
            arg[argc++] = token;
            token = strtok(NULL, " ");
        }
        arg[argc] = NULL;
        if (argc == 0)
            continue;

        // Fork and execute
        pid_t pid = fork();
        if (pid == 0)
        {
            if (background)
            {
                int devnull = open("/dev/null", O_RDONLY);
                dup2(devnull, STDIN_FILENO);
                close(devnull);
            }
            if (execvp(arg[0], arg) == -1)
                printf("Command not found!\n");
            exit(0);
        }
        else
        {
            if (!background)
                waitpid(pid, NULL, 0);
            else
            {
                printf("[%d] %d\n", job_number, pid);
                jobs[job_count].job_number = job_number;
                jobs[job_count].pid = pid;
                strncpy(jobs[job_count].command, arg[0], sizeof(jobs[job_count].command) - 1);
                jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0';
                strcpy(jobs[job_count].state, "Running");
                job_count++;
                job_number++;
            }
        }
    }
}

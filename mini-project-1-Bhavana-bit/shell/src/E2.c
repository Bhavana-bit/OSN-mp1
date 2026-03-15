#include "E2.h"

int is_valid_number(const char *s)
{
    if (!s || *s == '\0')
        return 0;
    if (*s == '-')
        s++;
    for (int i = 0; s[i]; i++)
        if (!isdigit(s[i]))
            return 0;
    return 1;
}

void execute_e2_shell()
{
    char input[2000];

    while (1)
    {
        pid_t bg_pid;
        int status;
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

        char *username = getenv("USER");
        char hostname[1024];
        gethostname(hostname, sizeof(hostname));
        char current_path[1024];
        getcwd(current_path, sizeof(current_path));
        char *home = getenv("HOME");
        char prompt[20000];

        strcpy(prompt, "<");
        strcat(prompt, username);
        strcat(prompt, "@");
        strcat(prompt, hostname);
        strcat(prompt, ":");
        if (home && strncmp(current_path, home, strlen(home)) == 0)
        {
            strcat(prompt, "~");
            strcat(prompt, current_path + strlen(home));
        }
        else
            strcat(prompt, current_path);
        strcat(prompt, ">");

        printf("%s", prompt);
        fflush(stdout);

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
            activities();
            continue;
        }

        char *arg[MAX_ARGS];
        int argc = 0;
        char *p = input;
        while (*p)
        {
            while (*p && (*p == ' ' || *p == '\t'))
                p++;
            if (!*p)
                break;
            arg[argc++] = p;
            while (*p && *p != ' ' && *p != '\t')
                p++;
            if (*p)
            {
                *p = '\0';
                p++;
            }
        }
        arg[argc] = NULL;
        if (argc == 0)
            continue;

        if (strcmp(arg[0], "ping") == 0)
        {
            if (argc != 3)
                printf("Invalid syntax!\n");
            else
                ping_command(arg[1], arg[2]);
            continue;
        }

        int background = 0;
        if (argc > 0 && strcmp(arg[argc - 1], "&") == 0)
        {
            background = 1;
            arg[argc - 1] = NULL;
            argc--;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            if (background)
            {
                int devnull = open("/dev/null", O_RDONLY);
                if (devnull >= 0)
                {
                    dup2(devnull, STDIN_FILENO);
                    close(devnull);
                }
            }
            execvp(arg[0], arg);
            printf("Command not found: %s\n", arg[0]);
            exit(1);
        }
        else if (pid > 0)
        {
            if (!background)
                waitpid(pid, NULL, 0);
            else
            {
                printf("[%d] %d\n", job_number, pid);
                if (job_count < MAX_JOBS)
                {
                    jobs[job_count].job_number = job_number;
                    jobs[job_count].pid = pid;
                    strncpy(jobs[job_count].command, input, sizeof(jobs[job_count].command) - 1);
                    jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0';
                    strcpy(jobs[job_count].state, "Running");
                    job_count++;
                    job_number++;
                }
                else
                    printf("Maximum job count reached\n");
            }
        }
        else
            printf("Failed to fork process\n");
    }
}

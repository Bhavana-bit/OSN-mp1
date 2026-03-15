#include "D2.h"

#include <stdio.h>    // for printf, fgets, stdin
#include <stdlib.h>   // for malloc, free, exit, getenv
#include <string.h>   // for strcpy, strcat, strlen, strncmp, strncpy
#include <unistd.h>   // for getcwd, gethostname, fork, execvp, dup2
#include <sys/wait.h> // for wait, waitpid, WNOHANG, WIFEXITED
#include <fcntl.h>
void execute_d2_shell()
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
                    break;
                }
            }
        }

        char *arg[100];
        char *username = getenv("USER");
        char hostname[1024];
        gethostname(hostname, sizeof(hostname));
        char current_path[1024];
        getcwd(current_path, sizeof(current_path));
        char *home = getenv("HOME");
        char prompt[20000];
        int args = 0, i = 0, start = 0, numofcmds = 0;

        // Build prompt
        strcpy(prompt, "<");
        strcat(prompt, username);
        strcat(prompt, "@");
        strcat(prompt, hostname);
        strcat(prompt, ":");
        if (strncmp(current_path, home, strlen(home)) == 0)
        {
            strcat(prompt, "~");
            strcat(prompt, current_path + strlen(home));
        }
        else
            strcat(prompt, current_path);
        strcat(prompt, ">");
        printf("%s\n", prompt);

        if (!fgets(input, sizeof(input), stdin))
            break;
        if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';
        if (strlen(input) == 0)
            continue;
        if (strcmp(input, "exit") == 0)
            break;

        // Remove trailing spaces/tabs
        int len = strlen(input) - 1;
        while (len >= 0 && (input[len] == ' ' || input[len] == '\t'))
            len--;
        input[len + 1] = '\0';

        // Check background job
        int is_background = 0;
        if (len >= 0 && input[len] == '&')
        {
            is_background = 1;
            input[len] = '\0';
        }

        // Split sequential commands by ';'
        char *seqcommands[20];
        int numofseq = 0;
        start = 0;
        for (int k = 0;; k++)
        {
            if (input[k] == ';' || input[k] == '\0')
            {
                int len = k - start;
                while (len > 0 && (input[start] == ' ' || input[start] == '\t'))
                {
                    start++;
                    len--;
                }
                while (len > 0 && (input[start + len - 1] == ' ' || input[start + len - 1] == '\t'))
                    len--;
                if (len > 0)
                {
                    seqcommands[numofseq] = malloc(len + 1);
                    strncpy(seqcommands[numofseq], &input[start], len);
                    seqcommands[numofseq][len] = '\0';
                    numofseq++;
                }
                if (input[k] == '\0')
                    break;
                start = k + 1;
            }
        }

        // Execute each sequential command
        for (int s = 0; s < numofseq; s++)
        {
            char *cmd = seqcommands[s];
            args = 0;
            i = 0;

            // Handle non-pipeline commands (you can add pipeline code similarly)
            char *inputfile = NULL;
            char *outputfile = NULL;
            int append = 0;

            while (cmd[i] != '\0')
            {
                while (cmd[i] == ' ' || cmd[i] == '\t')
                    i++;
                if (cmd[i] == '\0')
                    break;

                if (cmd[i] == '<')
                {
                    i++;
                    while (cmd[i] == ' ' || cmd[i] == '\t')
                        i++;
                    char *start = &cmd[i];
                    while (cmd[i] != ' ' && cmd[i] != '\t' && cmd[i] != '\0')
                        i++;
                    int len = &cmd[i] - start;
                    inputfile = malloc(len + 1);
                    strncpy(inputfile, start, len);
                    inputfile[len] = '\0';
                    continue;
                }

                if (cmd[i] == '>')
                {
                    if (cmd[i + 1] == '>')
                    {
                        append = 1;
                        i += 2;
                    }
                    else
                    {
                        append = 0;
                        i++;
                    }
                    while (cmd[i] == ' ' || cmd[i] == '\t')
                        i++;
                    char *start = &cmd[i];
                    while (cmd[i] != ' ' && cmd[i] != '\t' && cmd[i] != '\0')
                        i++;
                    int len = &cmd[i] - start;
                    outputfile = malloc(len + 1);
                    strncpy(outputfile, start, len);
                    outputfile[len] = '\0';
                    continue;
                }

                char *start = &cmd[i];
                while (cmd[i] != ' ' && cmd[i] != '\t' && cmd[i] != '\0')
                    i++;
                int len = &cmd[i] - start;
                arg[args] = malloc(len + 1);
                strncpy(arg[args], start, len);
                arg[args][len] = '\0';
                args++;
            }
            arg[args] = NULL;

            pid_t pid = fork();
            if (pid == 0)
            {
                if (inputfile)
                {
                    int file = open(inputfile, O_RDONLY);
                    if (file < 0)
                        printf("No such file or directory\n"), exit(1);
                    dup2(file, STDIN_FILENO);
                    close(file);
                }
                if (outputfile)
                {
                    int file;
                    if (append)
                        file = open(outputfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    else
                        file = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (file < 0)
                        printf("Unable to create file\n"), exit(1);
                    dup2(file, STDOUT_FILENO);
                    close(file);
                }

                if (is_background)
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
                if (!is_background)
                    wait(NULL);
                else
                {
                    printf("[%d] %d\n", job_number, pid);
                    jobs[job_count].job_number = job_number;
                    jobs[job_count].pid = pid;
                    strncpy(jobs[job_count].command, arg[0], sizeof(jobs[job_count].command) - 1);
                    jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0';
                    job_count++;
                    job_number++;
                }
            }

            for (int j = 0; j < args; j++)
                free(arg[j]);
            if (inputfile)
                free(inputfile);
            if (outputfile)
                free(outputfile);
        }

        for (int i = 0; i < numofseq; i++)
            free(seqcommands[i]);
    }
}

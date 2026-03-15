#include "C2.h"
#include <sys/wait.h>
#include <fcntl.h>
void execute_command_with_input(char *input)
{
    char *arg[100];
    char *inputfile = NULL;
    int args = 0;
    int i = 0;

    // --- Parse input ---
    while (input[i] != '\0')
    {
        while (input[i] == ' ' || input[i] == '\t')
            i++;
        if (input[i] == '\0')
            break;

        if (input[i] == '<')
        {
            i++;
            while (input[i] == ' ' || input[i] == '\t')
                i++;
            char *start = &input[i];
            while (input[i] != ' ' && input[i] != '\t' && input[i] != '\0')
                i++;
            int len = &input[i] - start;
            inputfile = malloc(len + 1);
            strncpy(inputfile, start, len);
            inputfile[len] = '\0';
            continue;
        }

        char *start = &input[i];
        while (input[i] != ' ' && input[i] != '\t' && input[i] != '\0')
            i++;
        int len = &input[i] - start;
        arg[args] = malloc(len + 1);
        strncpy(arg[args], start, len);
        arg[args][len] = '\0';
        args++;
    }
    arg[args] = NULL;

    // --- Fork & execute ---
    pid_t pid = fork();
    if (pid == 0)
    {
        if (inputfile != NULL)
        {
            int fd = open(inputfile, O_RDONLY);
            if (fd < 0)
            {
                printf("No such file or directory\n");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (execvp(arg[0], arg) == -1)
        {
            printf("Command not found!\n");
        }
        exit(0);
    }
    else
    {
        wait(NULL);
    }

    // --- Cleanup ---
    for (int j = 0; j < args; j++)
        free(arg[j]);
    if (inputfile != NULL)
        free(inputfile);
}

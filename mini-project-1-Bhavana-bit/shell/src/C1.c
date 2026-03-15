#include "C1.h"
#include <sys/wait.h>

void execute_command(char *input)
{
    char *arg[100];
    int args = 0;
    int i = 0;

    // Tokenize input into arguments
    while (input[i] != '\0')
    {
        while (input[i] == ' ' || input[i] == '\t')
            i++;
        if (input[i] == '\0')
            break;
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

    // Fork + exec
    pid_t pid = fork();
    if (pid == 0)
    {
        if (execvp(arg[0], arg) == -1)
            printf("Command not found!\n");
        exit(0);
    }
    else
    {
        wait(NULL);
    }

    // Free args
    for (int j = 0; j < args; j++)
        free(arg[j]);
}

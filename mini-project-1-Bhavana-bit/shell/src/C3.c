#include "C3.h"
#include <sys/wait.h>
#include <fcntl.h>

void printprompt1()
{
    char *username = getenv("USER");
    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char *home = getenv("HOME");

    printf("<%s@%s:", username, hostname);
    if (home && strncmp(cwd, home, strlen(home)) == 0)
        printf("~%s>", cwd + strlen(home));
    else
        printf("%s>", cwd);
    fflush(stdout);
}

void execute_command_with_redirection(char *input)
{
    char *args[100];
    char *inputfile = NULL;
    char *outputfile = NULL;
    int append = 0;
    int arg_count = 0;
    int i = 0;
    int input_redirect_count = 0;
    int output_redirect_count = 0;

    while (input[i] != '\0')
    {
        while (input[i] == ' ' || input[i] == '\t')
            i++;
        if (input[i] == '\0')
            break;

        if (input[i] == '<')
        {
            input_redirect_count++;
            if (input_redirect_count > 1)
            {
                printf("Syntax error: multiple input redirects\n");
                return;
            }
            i++;
            while (input[i] == ' ' || input[i] == '\t')
                i++;
            if (input[i] == '\0')
            {
                printf("Syntax error: no input file specified\n");
                return;
            }
            char *start = &input[i];
            while (input[i] && input[i] != ' ' && input[i] != '\t' && input[i] != '>' && input[i] != '<')
                i++;
            int len = &input[i] - start;
            inputfile = malloc(len + 1);
            strncpy(inputfile, start, len);
            inputfile[len] = '\0';
            continue;
        }

        if (input[i] == '>')
        {
            output_redirect_count++;
            if (output_redirect_count > 1)
            {
                printf("Syntax error: multiple output redirects\n");
                return;
            }
            if (input[i + 1] == '>')
            {
                append = 1;
                i += 2;
            }
            else
            {
                append = 0;
                i++;
            }
            while (input[i] == ' ' || input[i] == '\t')
                i++;
            if (input[i] == '\0')
            {
                printf("Syntax error: no output file specified\n");
                return;
            }
            char *start = &input[i];
            while (input[i] && input[i] != ' ' && input[i] != '\t' && input[i] != '<' && input[i] != '>')
                i++;
            int len = &input[i] - start;
            outputfile = malloc(len + 1);
            strncpy(outputfile, start, len);
            outputfile[len] = '\0';
            continue;
        }

        char *start = &input[i];
        while (input[i] && input[i] != ' ' && input[i] != '\t' && input[i] != '<' && input[i] != '>')
            i++;
        int len = &input[i] - start;
        args[arg_count] = malloc(len + 1);
        strncpy(args[arg_count], start, len);
        args[arg_count][len] = '\0';
        arg_count++;
    }
    args[arg_count] = NULL;

    if (arg_count == 0)
    {
        printf("No command to execute\n");
        if (inputfile)
            free(inputfile);
        if (outputfile)
            free(outputfile);
        return;
    }

    pid_t pid = fork();
    if (pid == 0)
    { // Child
        if (inputfile)
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
        if (outputfile)
        {
            int fd;
            if (append)
                fd = open(outputfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                printf("Unable to create file for writing\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(args[0], args);
        printf("Command not found!\n");
        exit(1);
    }
    else if (pid > 0)
    { // Parent
        wait(NULL);
    }
    else
    {
        perror("fork");
    }

    // Cleanup
    for (int j = 0; j < arg_count; j++)
        free(args[j]);
    if (inputfile)
        free(inputfile);
    if (outputfile)
        free(outputfile);
}

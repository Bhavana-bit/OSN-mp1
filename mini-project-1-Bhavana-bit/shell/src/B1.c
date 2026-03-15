#include "B1.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void hop(char *input, char *prev)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return;
    }

    char *args = input + 3; // skip "hop"
    while (*args == ' ' || *args == '\t')
        args++;

    if (*args == '\0')
    {
        char *home = getenv("HOME");
        if (home && chdir(home) == 0)
        {
            strcpy(prev, cwd);
        }
        else
        {
            printf("No such directory!\n");
        }
        return;
    }

    while (*args != '\0')
    {
        while (*args == ' ' || *args == '\t')
            args++;
        if (*args == '\0')
            break;

        char token[1024];
        int i = 0;
        while (*args != ' ' && *args != '\t' && *args != '\0')
            token[i++] = *args++;
        token[i] = '\0';

        if (strcmp(token, "~") == 0)
        {
            char *home = getenv("HOME");
            if (home && chdir(home) == 0)
            {
                strcpy(prev, cwd);
                getcwd(cwd, sizeof(cwd));
            }
            else
            {
                printf("No such directory!\n");
            }
        }
        else if (strcmp(token, "-") == 0)
        {
            if (prev[0] != '\0')
            {
                char temp[1024];
                strcpy(temp, cwd);
                if (chdir(prev) == 0)
                {
                    strcpy(prev, temp);
                    getcwd(cwd, sizeof(cwd));
                }
                else
                {
                    printf("No such directory!\n");
                }
            }
            // else: do nothing
        }
        else if (strcmp(token, "..") == 0)
        {
            if (chdir("..") == 0)
            {
                strcpy(prev, cwd);
                getcwd(cwd, sizeof(cwd));
            }
            else
            {
                printf("No such directory!\n");
            }
        }
        else if (strcmp(token, ".") == 0)
        {
            // Do nothing
        }
        else
        {
            if (chdir(token) == 0)
            {
                strcpy(prev, cwd);
                getcwd(cwd, sizeof(cwd));
            }
            else
            {
                printf("No such directory!\n");
            }
        }
    }
}

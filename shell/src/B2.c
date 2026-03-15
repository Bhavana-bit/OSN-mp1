#include "B2.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define MAX_TOKENS 20
#define MAX_LENGTH 256

void handle_reveal(char *input, char *prev)
{
    char *home = getenv("HOME");

    // Tokenize input
    char tokens[MAX_TOKENS][MAX_LENGTH];
    int count = 0, i = 0, j = 0;
    while (input[i] != '\0')
    {
        while (input[i] == ' ' || input[i] == '\t')
            i++;
        if (input[i] == '\0')
            break;

        j = 0;
        while (input[i] != '\0' && input[i] != ' ' && input[i] != '\t')
        {
            if (j < MAX_LENGTH - 1)
                tokens[count][j++] = input[i];
            i++;
        }
        tokens[count][j] = '\0';
        count++;
        if (count >= MAX_TOKENS)
            break;
    }

    if (count == 0 || strcmp(tokens[0], "reveal") != 0)
    {
        return; // not a reveal command
    }

    // Parse flags + argument
    char flags[8] = "";
    char arg[MAX_LENGTH] = "";
    int invalid = 0;
    for (int t = 1; t < count; t++)
    {
        if (tokens[t][0] == '-')
        {
            if (strcmp(tokens[t], "-") == 0)
            {
                if (arg[0] == '\0')
                    strcpy(arg, "-");
                else
                    invalid = 1;
            }
            else
            {
                for (int k = 1; tokens[t][k] != '\0'; k++)
                {
                    if (tokens[t][k] == 'a' || tokens[t][k] == 'l')
                    {
                        if (!strchr(flags, tokens[t][k]))
                        {
                            int len = strlen(flags);
                            flags[len] = tokens[t][k];
                            flags[len + 1] = '\0';
                        }
                    }
                    else
                    {
                        invalid = 1;
                        break;
                    }
                }
            }
        }
        else
        {
            if (arg[0] == '\0')
                strcpy(arg, tokens[t]);
            else
                invalid = 1;
        }
    }

    if (invalid)
    {
        printf("reveal: Invalid Syntax!\n");
        return;
    }

    // Resolve path
    char path[MAX_LENGTH];
    if (arg[0] == '\0')
        strcpy(path, ".");
    else if (strcmp(arg, "~") == 0)
        strcpy(path, home);
    else if (!strcmp(arg, ".") || !strcmp(arg, ".."))
        strcpy(path, arg);
    else if (!strcmp(arg, "-"))
    {
        if (strlen(prev) == 0)
        {
            printf("No such directory!\n");
            return;
        }
        strcpy(path, prev);
    }
    else
        strcpy(path, arg);

    // Open directory
    DIR *dir = opendir(path);
    if (!dir)
    {
        printf("No such directory!\n");
        return;
    }

    struct dirent *entry;
    char *files[1000];
    int n = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        int hidden = (entry->d_name[0] == '.');
        int show_hidden = strchr(flags, 'a') != NULL;
        if (!show_hidden && hidden)
            continue;

        files[n] = malloc(strlen(entry->d_name) + 1);
        strcpy(files[n], entry->d_name);
        n++;
    }
    closedir(dir);

    int long_list = strchr(flags, 'l') != NULL;
    for (int k = 0; k < n; k++)
    {
        if (long_list)
            printf("%s\n", files[k]);
        else
            printf("%s ", files[k]);
        free(files[k]);
    }
    if (!long_list)
        printf("\n");
}

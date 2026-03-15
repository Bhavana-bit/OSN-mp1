#include "D1.h"

void execute_d1_shell()
{
    while (1)
    {
        char input[2000];
        char *arg[100];
        char *username = getenv("USER");
        char hostname[1024];
        gethostname(hostname, sizeof(hostname));
        char current_path[1024];
        getcwd(current_path, sizeof(current_path));
        char *home = getenv("HOME");
        char prompt[20000];
        int args = 0, i = 0;
        char *commands[20];
        int numofcmds = 0;
        int start = 0;

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
        {
            strcat(prompt, current_path);
        }
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

        // -------- SEQUENTIAL COMMANDS (;) --------
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

        // Execute sequential commands
        for (int s = 0; s < numofseq; s++)
        {
            char *cmd = seqcommands[s];
            args = 0;
            i = 0;

            // -------- PIPE HANDLING --------
            if (strchr(cmd, '|') != NULL)
            {
                numofcmds = 0;
                start = 0;

                for (int k = 0;; k++)
                {
                    if (cmd[k] == '|' || cmd[k] == '\0')
                    {
                        int len = k - start;
                        while (len > 0 && (cmd[start] == ' ' || cmd[start] == '\t'))
                        {
                            start++;
                            len--;
                        }
                        while (len > 0 && (cmd[start + len - 1] == ' ' || cmd[start + len - 1] == '\t'))
                            len--;
                        if (len > 0)
                        {
                            commands[numofcmds] = malloc(len + 1);
                            strncpy(commands[numofcmds], &cmd[start], len);
                            commands[numofcmds][len] = '\0';
                            numofcmds++;
                        }
                        if (cmd[k] == '\0')
                            break;
                        start = k + 1;
                    }
                }

                int pipelen[2 * (numofcmds - 1)];
                for (int j = 0; j < numofcmds - 1; j++)
                    if (pipe(pipelen + j * 2) < 0)
                        perror("pipe"), exit(1);

                for (int j = 0; j < numofcmds; j++)
                {
                    pid_t pid = fork();
                    if (pid == 0)
                    {
                        if (j > 0)
                            dup2(pipelen[(j - 1) * 2], STDIN_FILENO);
                        if (j < numofcmds - 1)
                            dup2(pipelen[j * 2 + 1], STDOUT_FILENO);

                        for (int i = 0; i < 2 * (numofcmds - 1); i++)
                            close(pipelen[i]);

                        char *cmd_arg[100];
                        int cmd_args = 0, cmd_i = 0;
                        char *cmd_inputfile = NULL, *cmd_outputfile = NULL;
                        int cmd_append = 0;

                        while (commands[j][cmd_i] != '\0')
                        {
                            while (commands[j][cmd_i] == ' ' || commands[j][cmd_i] == '\t')
                                cmd_i++;
                            if (commands[j][cmd_i] == '\0')
                                break;

                            if (commands[j][cmd_i] == '<')
                            {
                                cmd_i++;
                                while (commands[j][cmd_i] == ' ' || commands[j][cmd_i] == '\t')
                                    cmd_i++;
                                char *start = &commands[j][cmd_i];
                                while (commands[j][cmd_i] != ' ' && commands[j][cmd_i] != '\t' && commands[j][cmd_i] != '\0')
                                    cmd_i++;
                                int len = &commands[j][cmd_i] - start;
                                cmd_inputfile = malloc(len + 1);
                                strncpy(cmd_inputfile, start, len);
                                cmd_inputfile[len] = '\0';
                                continue;
                            }

                            if (commands[j][cmd_i] == '>')
                            {
                                if (commands[j][cmd_i + 1] == '>')
                                {
                                    cmd_append = 1;
                                    cmd_i += 2;
                                }
                                else
                                {
                                    cmd_append = 0;
                                    cmd_i++;
                                }
                                while (commands[j][cmd_i] == ' ' || commands[j][cmd_i] == '\t')
                                    cmd_i++;
                                char *start = &commands[j][cmd_i];
                                while (commands[j][cmd_i] != ' ' && commands[j][cmd_i] != '\t' && commands[j][cmd_i] != '\0')
                                    cmd_i++;
                                int len = &commands[j][cmd_i] - start;
                                cmd_outputfile = malloc(len + 1);
                                strncpy(cmd_outputfile, start, len);
                                cmd_outputfile[len] = '\0';
                                continue;
                            }

                            char *start = &commands[j][cmd_i];
                            while (commands[j][cmd_i] != ' ' && commands[j][cmd_i] != '\t' && commands[j][cmd_i] != '\0')
                                cmd_i++;
                            int len = &commands[j][cmd_i] - start;
                            cmd_arg[cmd_args] = malloc(len + 1);
                            strncpy(cmd_arg[cmd_args], start, len);
                            cmd_arg[cmd_args][len] = '\0';
                            cmd_args++;
                        }
                        cmd_arg[cmd_args] = NULL;

                        if (cmd_inputfile)
                        {
                            int fd = open(cmd_inputfile, O_RDONLY);
                            if (fd < 0)
                                printf("No such file\n"), exit(1);
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                        }
                        if (cmd_outputfile)
                        {
                            int fd;
                            if (cmd_append)
                                fd = open(cmd_outputfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                            else
                                fd = open(cmd_outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            if (fd < 0)
                                printf("Unable to create file\n"), exit(1);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }

                        execvp(cmd_arg[0], cmd_arg);
                        printf("Command not found!\n");
                        exit(1);
                    }
                }

                for (int i = 0; i < 2 * (numofcmds - 1); i++)
                    close(pipelen[i]);
                for (int i = 0; i < numofcmds; i++)
                    wait(NULL);
                for (int i = 0; i < numofcmds; i++)
                    free(commands[i]);
            }
            // -------- SIMPLE COMMANDS --------
            else
            {
                args = 0;
                i = 0;
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
                            printf("Unable to create file for writing\n"), exit(1);
                        dup2(file, STDOUT_FILENO);
                        close(file);
                    }

                    if (execvp(arg[0], arg) == -1)
                        printf("Command not found!\n");
                    exit(0);
                }
                else
                    wait(NULL);

                for (int j = 0; j < args; j++)
                    free(arg[j]);
                if (inputfile)
                    free(inputfile);
                if (outputfile)
                    free(outputfile);
            }
        }

        for (int i = 0; i < numofseq; i++)
            free(seqcommands[i]);
    }
}

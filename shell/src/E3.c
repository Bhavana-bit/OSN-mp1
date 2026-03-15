#include "E3.h"

/*
void sigint_handler(int signum) {
    if (fg_pid > 0)
        kill(-fg_pid, SIGINT);
}

void sigtstp_handler(int signum) {
    if (fg_pid > 0)
        kill(-fg_pid, SIGTSTP);
}

void update_job_states() {
    for (int i = 0; i < job_count; i++) {
        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/stat", jobs[i].pid);

        FILE *file = fopen(path, "r");
        if (!file) {
            for (int j = i; j < job_count - 1; j++)
                jobs[j] = jobs[j + 1];
            job_count--;
            i--;
            continue;
        }

        char line[256];
        if (fgets(line, sizeof(line), file) == NULL) { fclose(file); continue; }
        fclose(file);

        char *token = strtok(line, " ");
        int field_count = 0;
        char state = ' ';
        while (token != NULL) {
            field_count++;
            if (field_count == 3) { state = token[0]; break; }
            token = strtok(NULL, " ");
        }

        if (state == 'T')
            strcpy(jobs[i].state, "Stopped");
        else
            strcpy(jobs[i].state, "Running");
    }
}

int compare_jobs(const void *a, const void *b) {
    const struct job *ja = (const struct Job *)a;
    const struct job *jb = (const struct Job *)b;
    return strcmp(ja->command, jb->command);
}*/

void activities_command()
{
    update_job_states();
    for (int i = 0; i < job_count - 1; i++)
    {
        for (int j = i + 1; j < job_count; j++)
        {
            if (strcmp(jobs[i].command, jobs[j].command) > 0)
            {
                struct job tmp = jobs[i];
                jobs[i] = jobs[j];
                jobs[j] = tmp;
            }
        }
    }
    for (int i = 0; i < job_count; i++)
        printf("[%d] : %s - %s\n", jobs[i].pid, jobs[i].command, jobs[i].state);
}

void execute_e3_shell()
{
    struct sigaction sa_int, sa_tstp;
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

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

        char cwd[1024], hostname[256];
        char *username = getenv("USER");
        getcwd(cwd, sizeof(cwd));
        gethostname(hostname, sizeof(hostname));
        char *home = getenv("HOME");

        printf("<%s@%s:", username, hostname);
        if (home && strncmp(cwd, home, strlen(home)) == 0)
            printf("~%s>", cwd + strlen(home));
        else
            printf("%s>", cwd);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            handle_eof();

        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';
        if (strlen(input) == 0)
            continue;
        if (strcmp(input, "exit") == 0)
            break;
        if (strcmp(input, "activities") == 0)
        {
            activities_command();
            continue;
        }

        int background = 0;
        int len = strlen(input);
        while (len > 0 && (input[len - 1] == ' ' || input[len - 1] == '\t'))
            len--;
        if (len > 0 && input[len - 1] == '&')
        {
            background = 1;
            input[len - 1] = '\0';
            while (len > 0 && (input[len - 1] == ' ' || input[len - 1] == '\t'))
                len--;
            input[len] = '\0';
        }

        char *argv[MAX_ARGS];
        int argc = 0;
        char *p = input;
        while (*p)
        {
            while (*p && (*p == ' ' || *p == '\t'))
                p++;
            if (!*p)
                break;
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t')
                p++;
            if (*p)
                *p++ = '\0';
        }
        argv[argc] = NULL;
        if (argc == 0)
            continue;

        pid_t pid = fork();
        if (pid == 0)
        {
            fg_pid = getpid();
            setpgid(0, 0);
            if (background)
            {
                int devnull = open("/dev/null", O_RDONLY);
                if (devnull >= 0)
                {
                    dup2(devnull, STDIN_FILENO);
                    close(devnull);
                }
            }
            execvp(argv[0], argv);
            perror("Command not found");
            exit(1);
        }
        else
        {
            if (!background)
            {
                fg_pid = pid;
                int status;
                waitpid(pid, &status, WUNTRACED);
                fg_pid = 0;
                if (WIFSTOPPED(status) && job_count < MAX_JOBS)
                {
                    jobs[job_count].job_number = job_number++;
                    jobs[job_count].pid = pid;
                    strncpy(jobs[job_count].command, input, MAX_CMD_LEN - 1);
                    jobs[job_count].command[MAX_CMD_LEN - 1] = '\0';
                    strcpy(jobs[job_count].state, "Stopped");
                    job_count++;
                }
            }
            else if (job_count < MAX_JOBS)
            {
                jobs[job_count].job_number = job_number++;
                jobs[job_count].pid = pid;
                strncpy(jobs[job_count].command, input, MAX_CMD_LEN - 1);
                jobs[job_count].command[MAX_CMD_LEN - 1] = '\0';
                strcpy(jobs[job_count].state, "Running");
                printf("[%d] %d\n", jobs[job_count].job_number, pid);
                job_count++;
            }
        }
    }
}

#include "E4.h"
/*
struct job jobs[MAX_JOBS];
int job_count = 0;
int job_number = 1;

void inttostr(int num, char *str)
{
    int i = 0;
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    int temp = num;
    while (temp > 0)
    {
        str[i++] = (temp % 10) + '0';
        temp = temp/10;
    }
    str[i] = '\0';
    for (int j = 0; j < i / 2; j++)
    {
        char c = str[j];
        str[j] = str[i - j - 1];
        str[i-j-1] = c;
    }
}

void update_job_states()
{
    for (int i = 0; i < job_count; i++)
    {
        char path[64];
        char pid_str[20];
        inttostr(jobs[i].pid, pid_str);
        strcpy(path, "/proc/");
        strcat(path, pid_str);
        strcat(path, "/stat");

        FILE *file = fopen(path, "r");
        if (!file)
        {
            for (int j = i; j < job_count-1; j++)
            {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            i--;
            continue;
        }

        char line[256];
        if (fgets(line, sizeof(line), file) == NULL) {
            fclose(file);
            continue;
        }
        fclose(file);

        char *token = strtok(line, " ");
        int field_count = 0;
        char state = ' ';
        while (token != NULL) {
            field_count++;
            if (field_count == 3) {
                state = token[0];
                break;
            }
            token = strtok(NULL, " ");
        }

        if (state == 'T') strcpy(jobs[i].state, "Stopped");
        else strcpy(jobs[i].state, "Running");
    }
}

int compare_jobs(const void *a, const void *b)
{
    const struct job *ja = (const struct job *)a;
    const struct job *jb = (const struct job *)b;
    return strcmp(ja->command, jb->command);
}

void activities()
{
    update_job_states();
    qsort(jobs, job_count, sizeof(struct job), compare_jobs);
    for (int i = 0; i < job_count; i++)
    {
        printf("[%d] : %s - %s\n", jobs[i].pid, jobs[i].command, jobs[i].state);
    }
}

int is_number(const char *s) {
    if (s == NULL || *s == '\0') return 0;
    for (int i = 0; s[i]; i++) {
        if (!isdigit(s[i])) return 0;
    }
    return 1;
}*/
/*
int find_job_by_number(int job_num) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_number == job_num) return i;
    }
    return -1;
}

int get_most_recent_job() {
    if (job_count == 0) return -1;
    int max_job_num = jobs[0].job_number;
    int index = 0;
    for (int i = 1; i < job_count; i++) {
        if (jobs[i].job_number > max_job_num) {
            max_job_num = jobs[i].job_number;
            index = i;
        }
    }
    return index;
}

void fg_command(char *job_num_str) {
    int job_index;
    if (job_num_str == NULL) job_index = get_most_recent_job();
    else {
        if (!is_number(job_num_str)) { printf("No such job\n"); return; }
        int job_num = atoi(job_num_str);
        job_index = find_job_by_number(job_num);
    }
    if (job_index == -1) { printf("No such job\n"); return; }

    struct job *job = &jobs[job_index];
    printf("%s\n", job->command);
    if (strcmp(job->state, "Stopped") == 0) {
        kill(job->pid, SIGCONT);
        strcpy(job->state, "Running");
    }
    int status;
    waitpid(job->pid, &status, WUNTRACED);
    if (WIFSTOPPED(status)) strcpy(job->state, "Stopped");
    else {
        for (int i = job_index; i < job_count - 1; i++) jobs[i] = jobs[i + 1];
        job_count--;
    }
}

void bg_command(char *job_num_str) {
    int job_index;
    if (job_num_str == NULL) job_index = get_most_recent_job();
    else {
        if (!is_number(job_num_str)) { printf("No such job\n"); return; }
        int job_num = atoi(job_num_str);
        job_index = find_job_by_number(job_num);
    }
    if (job_index == -1) { printf("No such job\n"); return; }

    struct job *job = &jobs[job_index];
    if (strcmp(job->state, "Running") == 0) { printf("Job already running\n"); return; }
    kill(job->pid, SIGCONT);
    strcpy(job->state, "Running");
    printf("[%d] %s &\n", job->job_number, job->command);
}
*/
void execute_e4_shell()
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
                    {
                        printf("%s with pid %d exited normally\n", jobs[j].command, bg_pid);
                    }
                    else
                    {
                        printf("%s with pid %d exited abnormally\n", jobs[j].command, bg_pid);
                    }
                    for (int k = j; k < job_count - 1; k++)
                    {
                        jobs[k] = jobs[k + 1];
                    }
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
        if (home != NULL && strncmp(current_path, home, strlen(home)) == 0)
        {
            strcat(prompt, "~");
            strcat(prompt, current_path + strlen(home));
        }
        else
        {
            strcat(prompt, current_path);
        }
        strcat(prompt, ">");
        printf("%s", prompt);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
        {
            break;
        }
        if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
        {
            input[strlen(input) - 1] = '\0';
        }
        if (strlen(input) == 0)
        {
            continue;
        }
        if (strcmp(input, "exit") == 0)
        {
            break;
        }
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
            if (*p == '\0')
            {
                break;
            }
            arg[argc++] = p;
            while (*p && *p != ' ' && *p != '\t')
            {
                p++;
            }
            if (*p)
            {
                *p = '\0';
                p++;
            }
        }
        arg[argc] = NULL;
        if (argc == 0)
        {
            continue;
        }

        if (strcmp(arg[0], "fg") == 0)
        {
            fg_command(argc > 1 ? arg[1] : NULL);
            continue;
        }
        if (strcmp(arg[0], "bg") == 0)
        {
            bg_command(argc > 1 ? arg[1] : NULL);
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
            {
                waitpid(pid, NULL, 0);
            }
            else if (job_count < MAX_JOBS)
            {
                jobs[job_count].job_number = job_number;
                jobs[job_count].pid = pid;
                strncpy(jobs[job_count].command, input, sizeof(jobs[job_count].command) - 1);
                jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0';
                strcpy(jobs[job_count].state, "Running");
                printf("[%d] %d\n", job_number, pid);
                job_count++;
                job_number++;
            }
            else
            {
                printf("Maximum job count reached\n");
            }
        }
        else
        {
            printf("Failed to fork process\n");
        }
    }
}

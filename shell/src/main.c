#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <regex.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#define PROMPT_SIZE 1024
#define MAX_TOKENS 20
#define MAX_LENGTH 256
#define MAX_LOG 15
#define MAX_JOBS 100
#define MAX_ARGS 100
#define MAX_CMD_LEN 256
#define LOG_FILE "log.txt"
#include "A3.h"
#include "B1.h"
#include "B2.h"
#include "B3.h"
#include "C1.h"
#include "C2.h"
#include "C3.h"
#include "C4.h"
#include "D1.h"
#include "D2.h"
#include "E1.h"
#include "E2.h"
#include "E3.h"
#include "E4.h"

// Structure for job control

// Global variables
char log_arr[MAX_LOG][1024];
int log_count = 0;
struct job jobs[MAX_JOBS];
int job_count = 0;
int job_number = 1;
pid_t fg_pid = 0;
char prev[1024] = "";

// Function declarations
void build_prompt(char *prompt, int size);
void inttostr(int num, char *str);
void update_job_states();
int compare_jobs(const void *a, const void *b);
void activities();
void sigint_handler(int signum);
void sigtstp_handler(int signum);
void handle_eof();
void load_log_history();
void save_log_history();
void add_to_history(const char *input);
void print_history();
void purge_history();
void execute_from_history(int index);
void hop_command(char *args);
void reveal_command(char *args);
void ping_command(char *pid_str, char *sig_str);
void fg_command(char *job_num_str);
void bg_command(char *job_num_str);
int is_number(const char *s);
int find_job_by_number(int job_num);
int get_most_recent_job();
void execute_command(char *input);
int parse_and_execute(char *input, int background);
int handle_pipes(char *input);
int handle_redirection(char *input, char *input_file, char *output_file, int *append);

int main()
{
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    load_log_history();
    while (1)
    {
        char prompt[PROMPT_SIZE];
        build_prompt(prompt, sizeof(prompt));
        printf("%s", prompt);
        fflush(stdout);

        char input[1024];
        if (!fgets(input, sizeof(input), stdin))
        {
            handle_eof();
        }
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
        {
            input[len - 1] = '\0';
        }
        if (strlen(input) == 0)
        {
            continue;
        }

        if (strcmp(input, "exit") == 0)
        {
            printf("logout\n");
            break;
        }
        add_to_history(input);

        int background = 0;
        len = strlen(input);
        if (len > 0 && input[len - 1] == '&')
        {
            background = 1;
            input[len - 1] = '\0';
            while (len > 1 && (input[len - 2] == ' ' || input[len - 2] == '\t'))
            {
                input[len - 2] = '\0';
                len--;
            }
        }

        if (strncmp(input, "hop", 3) == 0)
        {
            hop(input, prev);
            continue;
        }

        if (strncmp(input, "reveal", 6) == 0)
        {
            reveal_command(input + 6);
            continue;
        }

        if (strncmp(input, "log", 3) == 0)
        {
            if (strcmp(input, "log") == 0)
            {
                print_history();
            }
            else if (strcmp(input, "log purge") == 0)
            {
                purge_history();
            }
            else if (strncmp(input, "log execute ", 12) == 0)
            {
                char *idx_str = input + 12;
                if (is_number(idx_str))
                {
                    int idx = atoi(idx_str);
                    execute_from_history(idx);
                }
                else
                {
                    printf("Invalid index: %s\n", idx_str);
                }
            }
            else
            {
                printf("Invalid log command: %s\n", input);
            }
            continue;
        }

        if (strcmp(input, "activities") == 0)
        {
            activities();
            continue;
        }

        if (strncmp(input, "ping", 4) == 0)
        {
            char *args = input + 4;
            while (*args == ' ' || *args == '\t')
                args++;
            char *pid_str = strtok(args, " ");
            char *sig_str = strtok(NULL, " ");
            if (pid_str && sig_str)
            {
                ping_command(pid_str, sig_str);
            }
            else
            {
                printf("Usage: ping PID SIGNAL\n");
            }
            continue;
        }

        if (strncmp(input, "fg", 2) == 0)
        {
            char *arg = input + 2;
            while (*arg == ' ' || *arg == '\t')
                arg++;
            if (*arg == '\0')
                arg = NULL;
            fg_command(arg);
            continue;
        }

        if (strncmp(input, "bg", 2) == 0)
        {
            char *arg = input + 2;
            while (*arg == ' ' || *arg == '\t')
                arg++;
            if (*arg == '\0')
                arg = NULL;
            bg_command(arg);
            continue;
        }

        parse_and_execute(input, background);
    }

    save_log_history();
}

void print_prompt()
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

void build_prompt(char *prompt, int size)
{
    char *username = getenv("USER");
    if (!username)
        username = "unknown";

    char hostname[1024];
    gethostname(hostname, sizeof(hostname));

    char current_path[1024];
    getcwd(current_path, sizeof(current_path));

    char *home = getenv("HOME");

    // Add timestamp to prompt
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", t);

    snprintf(prompt, size, "[%s] <%s@%s:", timestamp, username, hostname);

    if (home && strncmp(current_path, home, strlen(home)) == 0)
    {
        strcat(prompt, "~");
        strcat(prompt, current_path + strlen(home));
    }
    else
    {
        strcat(prompt, current_path);
    }
    strcat(prompt, "> ");
}

void inttostr(int num, char *str)
{
    sprintf(str, "%d", num);
}

void update_job_states()
{
    for (int i = 0; i < job_count; i++)
    {
        char path[64];
        char pid_str[20];
        inttostr(jobs[i].pid, pid_str);
        snprintf(path, sizeof(path), "/proc/%s/stat", pid_str);

        FILE *file = fopen(path, "r");
        if (!file)
        {
            // Process no longer exists, remove from job list
            for (int j = i; j < job_count - 1; j++)
            {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            i--;
            continue;
        }

        char state;
        fscanf(file, "%*d %*s %c", &state);
        fclose(file);

        if (state == 'T')
        {
            strcpy(jobs[i].state, "Stopped");
        }
        else
        {
            strcpy(jobs[i].state, "Running");
        }
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

    if (job_count == 0)
    {
        printf("No background jobs\n");
        return;
    }

    printf("Job ID\tPID\tState\t\tCommand\n");
    for (int i = 0; i < job_count; i++)
    {
        printf("[%d]\t%d\t%s\t\t%s\n",
               jobs[i].job_number, jobs[i].pid, jobs[i].state, jobs[i].command);
    }
}

void sigint_handler(int signum)
{
    if (fg_pid > 0)
    {
        kill(-fg_pid, SIGINT);
    }
    else
    {
        printf("\n");
        char prompt[PROMPT_SIZE];
        build_prompt(prompt, sizeof(prompt));
        printf("%s", prompt);
        fflush(stdout);
    }
}

void sigtstp_handler(int signum)
{
    if (fg_pid > 0)
    {
        kill(-fg_pid, SIGTSTP);
    }
}
/*
void handle_eof() {
    printf("\nUse 'exit' to quit the shell\n");
    char prompt[PROMPT_SIZE];
    build_prompt(prompt, sizeof(prompt));
    printf("%s", prompt);
    fflush(stdout);
}*/
void handle_eof()
{
    printf("\nlogout\n"); // print logout
    save_log_history();   // save history
    exit(0);              // terminate shell
}

void load_log_history()
{
    FILE *fp = fopen(LOG_FILE, "r");
    if (fp)
    {
        while (fgets(log_arr[log_count], 1024, fp) && log_count < MAX_LOG)
        {
            size_t len = strlen(log_arr[log_count]);
            if (len > 0 && log_arr[log_count][len - 1] == '\n')
            {
                log_arr[log_count][len - 1] = '\0';
            }
            log_count++;
        }
        fclose(fp);
    }
}

void save_log_history()
{
    FILE *f = fopen(LOG_FILE, "w");
    if (f)
    {
        int start = log_count > MAX_LOG ? log_count - MAX_LOG : 0;
        for (int i = start; i < log_count; i++)
        {
            fprintf(f, "%s\n", log_arr[i % MAX_LOG]);
        }
        fclose(f);
    }
}

void add_to_history(const char *input)
{
    // Don't add duplicate consecutive commands
    if (log_count > 0 && strcmp(log_arr[(log_count - 1) % MAX_LOG], input) == 0)
    {
        return;
    }

    strcpy(log_arr[log_count % MAX_LOG], input);
    log_count++;

    // Save history after each command
    save_log_history();
}

void print_history()
{
    if (log_count == 0)
    {
        printf("No command history\n");
        return;
    }

    int start = log_count > MAX_LOG ? log_count - MAX_LOG : 0;
    printf("Command History:\n");
    for (int i = start; i < log_count; i++)
    {
        printf("%d: %s\n", i - start + 1, log_arr[i % MAX_LOG]);
    }
}

void purge_history()
{
    log_count = 0;
    save_log_history();
    printf("Command history cleared\n");
}

void execute_from_history(int index)
{
    if (index < 1 || index > MAX_LOG)
    {
        printf("Invalid history index: %d\n", index);
        return;
    }

    int actual_index = (log_count - index) % MAX_LOG;
    if (actual_index < 0)
    {
        printf("No command at index: %d\n", index);
        return;
    }

    printf("Executing: %s\n", log_arr[actual_index]);
    parse_and_execute(log_arr[actual_index], 0);
}
/*
void hop_command(char *args) {
    static char prev[1024] = "";  // keep previous directory
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return;
    }

    char *token = strtok(args, " ");

    // No arguments -> go to home directory
    if (!token) {
        char *home = getenv("HOME");
        if (home && chdir(home) == 0) {
            strcpy(prev, cwd);
        } else {
            printf("No such directory: %s\n", home);
        }
        return;
    }

    while (token) {
        if (strcmp(token, "~") == 0) {
            char *home = getenv("HOME");
            if (home && chdir(home) == 0) {
                strcpy(prev, cwd);
                getcwd(cwd, sizeof(cwd));
            } else {
                printf("No such directory: %s\n", home);
            }

        } else if (strcmp(token, "-") == 0) {
            if (strlen(prev) > 0) {
                char temp[1024];
                getcwd(temp, sizeof(temp));   // save current dir
                if (chdir(prev) == 0) {
                    strcpy(prev, temp);       // swap prev and cwd
                    getcwd(cwd, sizeof(cwd));
                } else {
                    printf("No such directory: %s\n", prev);
                }
            } else {
                // prev is empty -> do nothing
                // prompt stays the same
            }

        } else if (strcmp(token, "..") == 0) {
            if (chdir("..") == 0) {
                strcpy(prev, cwd);
                getcwd(cwd, sizeof(cwd));
            } else {
                perror("chdir");
            }

        } else if (strcmp(token, ".") == 0) {
            // do nothing

        } else {
            if (chdir(token) == 0) {
                strcpy(prev, cwd);
                getcwd(cwd, sizeof(cwd));
            } else {
                printf("No such directory!\n");
            }
        }

        token = strtok(NULL, " ");
    }
}
*/
void hop_command(char *args)
{
    char *token = strtok(args, " ");
    char cwd[1024], temp[1024];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return;
    }

    if (!token)
    {
        char *home = getenv("HOME");
        if (home && chdir(home) == 0)
        {
            strcpy(prev, cwd);
        }
        else
        {
            printf("No such directory: %s\n", home);
        }
        return;
    }

    while (token)
    {
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
                printf("No such directory: %s\n", home);
            }
        }
        else if (strcmp(token, "-") == 0)
        {
            if (prev[0] != '\0')
            {
                strcpy(temp, cwd);
                if (chdir(prev) == 0)
                {
                    strcpy(prev, temp);
                    getcwd(cwd, sizeof(cwd));
                }
                else
                {
                    printf("No such directory: %s\n", prev);
                }
            }
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
                perror("chdir");
            }
        }
        else if (strcmp(token, ".") == 0)
        {
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

        token = strtok(NULL, " ");
    }
}

void reveal_command(char *args)
{
    char flags[8] = {0};
    char path[1024] = {0};
    char *token = strtok(args, " ");

    // Parse flags and path
    while (token)
    {
        if (token[0] == '-')
        {
            for (int i = 1; token[i]; i++)
            {
                if (token[i] == 'a' || token[i] == 'l')
                {
                    if (!strchr(flags, token[i]))
                    {
                        int len = strlen(flags);
                        flags[len] = token[i];
                        flags[len + 1] = '\0';
                    }
                }
                else
                {
                    printf("Invalid flag: -%c\n", token[i]);
                    return;
                }
            }
        }
        else
        {
            if (path[0] == '\0')
            {
                strcpy(path, token);
            }
            else
            {
                printf("Too many arguments\n");
                return;
            }
        }
        token = strtok(NULL, " ");
    }

    // Default to current directory if no path specified
    if (path[0] == '\0')
    {
        strcpy(path, ".");
    }

    // Handle special path names
    if (strcmp(path, "~") == 0)
    {
        char *home = getenv("HOME");
        if (home)
        {
            strcpy(path, home);
        }
        else
        {
            printf("HOME environment variable not set\n");
            return;
        }
    }
    else if (strcmp(path, "-") == 0)
    {
        if (strlen(prev) > 0)
        {
            strcpy(path, prev);
        }
        else
        {
            printf("No previous directory\n");
            return;
        }
    }

    DIR *dir = opendir(path);
    if (!dir)
    {
        printf("No such directory: %s\n", path);
        return;
    }

    struct dirent *entry;
    char *files[1000];
    int n = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        // Skip hidden files unless -a flag is set
        if (entry->d_name[0] == '.' && !strchr(flags, 'a'))
        {
            continue;
        }

        files[n] = malloc(strlen(entry->d_name) + 1);
        strcpy(files[n], entry->d_name);
        n++;
    }
    closedir(dir);

    // Sort files alphabetically
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (strcmp(files[i], files[j]) > 0)
            {
                char *temp = files[i];
                files[i] = files[j];
                files[j] = temp;
            }
        }
    }

    // Print files
    int line_output = strchr(flags, 'l') != NULL;
    for (int i = 0; i < n; i++)
    {
        if (line_output)
        {
            printf("%s\n", files[i]);
        }
        else
        {
            printf("%s  ", files[i]);
        }
        free(files[i]);
    }

    if (!line_output)
    {
        printf("\n");
    }
}

void ping_command(char *pid_str, char *sig_str)
{
    if (!is_number(pid_str) || !is_number(sig_str))
    {
        printf("Invalid arguments: PID and signal must be numbers\n");
        return;
    }

    pid_t pid = atoi(pid_str);
    int signal_num = atoi(sig_str);

    if (kill(pid, signal_num) == 0)
    {
        printf("Sent signal %d to process %d\n", signal_num, pid);
    }
    else
    {
        perror("kill");
    }
}

void fg_command(char *job_num_str)
{
    int job_index;

    if (job_num_str == NULL)
    {
        job_index = get_most_recent_job();
    }
    else
    {
        if (!is_number(job_num_str))
        {
            printf("Invalid job number: %s\n", job_num_str);
            return;
        }
        int job_num = atoi(job_num_str);
        job_index = find_job_by_number(job_num);
    }

    if (job_index == -1)
    {
        printf("No such job\n");
        return;
    }

    printf("%s\n", jobs[job_index].command);

    if (strcmp(jobs[job_index].state, "Stopped") == 0)
    {
        if (kill(jobs[job_index].pid, SIGCONT) != 0)
        {
            perror("kill");
            return;
        }
        strcpy(jobs[job_index].state, "Running");
    }

    fg_pid = jobs[job_index].pid;
    int status;
    waitpid(jobs[job_index].pid, &status, WUNTRACED);
    fg_pid = 0;

    if (WIFSTOPPED(status))
    {
        strcpy(jobs[job_index].state, "Stopped");
    }
    else
    {
        // Remove job from list
        for (int i = job_index; i < job_count - 1; i++)
        {
            jobs[i] = jobs[i + 1];
        }
        job_count--;
    }
}

void bg_command(char *job_num_str)
{
    int job_index;

    if (job_num_str == NULL)
    {
        job_index = get_most_recent_job();
    }
    else
    {
        if (!is_number(job_num_str))
        {
            printf("Invalid job number: %s\n", job_num_str);
            return;
        }
        int job_num = atoi(job_num_str);
        job_index = find_job_by_number(job_num);
    }

    if (job_index == -1)
    {
        printf("No such job\n");
        return;
    }

    if (strcmp(jobs[job_index].state, "Running") == 0)
    {
        printf("Job is already running\n");
        return;
    }

    if (kill(jobs[job_index].pid, SIGCONT) != 0)
    {
        perror("kill");
        return;
    }

    strcpy(jobs[job_index].state, "Running");
    printf("[%d] %s &\n", jobs[job_index].job_number, jobs[job_index].command);
}

int is_number(const char *s)
{
    if (!s || !*s)
        return 0;
    for (int i = 0; s[i]; i++)
    {
        if (!isdigit(s[i]))
            return 0;
    }
    return 1;
}

int find_job_by_number(int job_num)
{
    for (int i = 0; i < job_count; i++)
    {
        if (jobs[i].job_number == job_num)
        {
            return i;
        }
    }
    return -1;
}

int get_most_recent_job()
{
    if (job_count == 0)
        return -1;

    int max_job_num = jobs[0].job_number;
    int index = 0;

    for (int i = 1; i < job_count; i++)
    {
        if (jobs[i].job_number > max_job_num)
        {
            max_job_num = jobs[i].job_number;
            index = i;
        }
    }

    return index;
}

int parse_and_execute(char *input, int background)
{
    // Check for pipes
    if (strchr(input, '|') != NULL)
    {
        return handle_pipes(input);
    }

    char input_file[1024] = {0};
    char output_file[1024] = {0};
    int append = 0;

    // Check for redirection
    if (handle_redirection(input, input_file, output_file, &append) != 0)
    {
        return -1;
    }

    // Tokenize command
    char *args[MAX_ARGS];
    int argc = 0;
    char *token = strtok(input, " ");

    while (token && argc < MAX_ARGS - 1)
    {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    args[argc] = NULL;

    if (argc == 0)
    {
        return 0;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        if (input_file[0] != '\0')
        {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0)
            {
                perror("open");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (output_file[0] != '\0')
        {
            int flags = O_WRONLY | O_CREAT;
            flags |= (append ? O_APPEND : O_TRUNC);

            int fd = open(output_file, flags, 0644);
            if (fd < 0)
            {
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (background)
        {
            int devnull = open("/dev/null", O_RDONLY);
            if (devnull >= 0)
            {
                dup2(devnull, STDIN_FILENO);
                close(devnull);
            }
        }

        execvp(args[0], args);
        fprintf(stderr, "Command not found!\n"); // ✅ This is the fix
        exit(1);
    }

    else if (pid > 0)
    {
        // Parent process
        if (!background)
        {
            // Foreground process
            fg_pid = pid;
            int status;
            waitpid(pid, &status, WUNTRACED);
            fg_pid = 0;

            if (WIFSTOPPED(status))
            {
                // Add to job list if stopped
                if (job_count < MAX_JOBS)
                {
                    jobs[job_count].job_number = job_number++;
                    jobs[job_count].pid = pid;
                    strncpy(jobs[job_count].command, input, MAX_CMD_LEN - 1);
                    jobs[job_count].command[MAX_CMD_LEN - 1] = '\0';
                    strcpy(jobs[job_count].state, "Stopped");
                    job_count++;
                }
            }
        }
        else
        {
            // Background process
            if (job_count < MAX_JOBS)
            {
                jobs[job_count].job_number = job_number++;
                jobs[job_count].pid = pid;
                strncpy(jobs[job_count].command, input, MAX_CMD_LEN - 1);
                jobs[job_count].command[MAX_CMD_LEN - 1] = '\0';
                strcpy(jobs[job_count].state, "Running");
                printf("[%d] %d\n", jobs[job_count].job_number, pid);
                job_count++;
            }
            else
            {
                printf("Maximum job count reached\n");
            }
        }
        return 0;
    }
    else
    {
        perror("fork");
        return -1;
    }
}

int handle_pipes(char *input)
{
    char *commands[10];
    int num_commands = 0;

    // Split input into commands
    char *token = strtok(input, "|");
    while (token && num_commands < 10)
    {
        // Remove leading/trailing spaces
        while (*token == ' ' || *token == '\t')
            token++;
        int len = strlen(token);
        while (len > 0 && (token[len - 1] == ' ' || token[len - 1] == '\t'))
        {
            token[len - 1] = '\0';
            len--;
        }

        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    if (num_commands < 2)
    {
        printf("Invalid pipe syntax\n");
        return -1;
    }

    int pipes[2];
    int prev_pipe_in = -1;
    pid_t pids[10];
    int num_pids = 0;

    for (int i = 0; i < num_commands; i++)
    {
        if (i < num_commands - 1)
        {
            if (pipe(pipes) < 0)
            {
                perror("pipe");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            if (i > 0)
            {
                // Not the first command - read from previous pipe
                dup2(prev_pipe_in, STDIN_FILENO);
                close(prev_pipe_in);
            }

            if (i < num_commands - 1)
            {
                // Not the last command - write to next pipe
                dup2(pipes[1], STDOUT_FILENO);
                close(pipes[1]);
                close(pipes[0]);
            }

            // Execute the command
            char *args[MAX_ARGS];
            int argc = 0;
            char *cmd_token = strtok(commands[i], " ");

            while (cmd_token && argc < MAX_ARGS - 1)
            {
                args[argc++] = cmd_token;
                cmd_token = strtok(NULL, " ");
            }
            args[argc] = NULL;

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
        else if (pid > 0)
        {
            // Parent process
            pids[num_pids++] = pid;

            if (i > 0)
            {
                close(prev_pipe_in);
            }

            if (i < num_commands - 1)
            {
                close(pipes[1]);
                prev_pipe_in = pipes[0];
            }
        }
        else
        {
            perror("fork");
            return -1;
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_pids; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    return 0;
}

int handle_redirection(char *input, char *input_file, char *output_file, int *append)
{
    char *in_pos = strchr(input, '<');
    char *out_pos = strchr(input, '>');

    if (in_pos && out_pos && in_pos > out_pos)
    {
        // Handle case where output redirection comes before input
        printf("Syntax error: input redirection must come before output redirection\n");
        return -1;
    }

    // Process input redirection
    if (in_pos)
    {
        *in_pos = '\0'; // Terminate the command string
        char *file_start = in_pos + 1;
        while (*file_start == ' ' || *file_start == '\t')
            file_start++;

        if (*file_start == '\0')
        {
            printf("Syntax error: no input file specified\n");
            return -1;
        }

        char *file_end = file_start;
        while (*file_end && *file_end != ' ' && *file_end != '\t' && *file_end != '>')
            file_end++;

        strncpy(input_file, file_start, file_end - file_start);
        input_file[file_end - file_start] = '\0';

        // Remove the redirection part from the original string
        if (*file_end)
        {
            memmove(in_pos, file_end, strlen(file_end) + 1);
        }
        else
        {
            *in_pos = '\0';
        }
    }

    // Process output redirection
    if (out_pos)
    {
        // Check for append (>>)
        if (*(out_pos + 1) == '>')
        {
            *append = 1;
            *out_pos = '\0';
            out_pos++;
            *out_pos = '\0';
        }
        else
        {
            *append = 0;
            *out_pos = '\0';
        }

        char *file_start = out_pos + 1;
        while (*file_start == ' ' || *file_start == '\t')
            file_start++;

        if (*file_start == '\0')
        {
            printf("Syntax error: no output file specified\n");
            return -1;
        }

        char *file_end = file_start;
        while (*file_end && *file_end != ' ' && *file_end != '\t')
            file_end++;

        strncpy(output_file, file_start, file_end - file_start);
        output_file[file_end - file_start] = '\0';

        // Remove the redirection part from the original string
        if (*file_end)
        {
            memmove(out_pos, file_end, strlen(file_end) + 1);
        }
        else
        {
            *out_pos = '\0';
        }
    }

    return 0;
}

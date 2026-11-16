#define _POSIX_C_SOURCE 200809L
/* TechShell - CSC 222 shell project
 * Author: Mohammad Jahiruddin Ansari
 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 1024

typedef struct
{
    char *args[MAX_ARGS + 1];
    int count;
    char *input;
    char *output;
} Command;

static void error_message(int code)
{
    fprintf(stderr, "Error %d (%s)\n", code, strerror(code));
}

static void clear_command(Command *cmd)
{
    for (int i = 0; i < cmd->count; i++)
        free(cmd->args[i]);
    free(cmd->input);
    free(cmd->output);
    memset(cmd, 0, sizeof(*cmd));
}

/* Read one word, respecting single/double quotes and backslash escapes.
 * Returns 1 for a word, 0 for end/operator, -1 for invalid input. */
static int read_word(const char **cursor, char **result)
{
    const char *p = *cursor;
    size_t cap = strlen(p) + 1, len = 0;
    char *word = malloc(cap);
    int quote = 0, started = 0;
    if (!word)
        return -1;
    while (*p)
    {
        unsigned char c = (unsigned char)*p;
        if (!quote && (isspace(c) || c == '<' || c == '>'))
            break;
        started = 1;
        if (c == '\\' && p[1])
        {
            word[len++] = p[1];
            p += 2;
        }
        else if (c == '\\' && !p[1])
        {
            free(word);
            return -1;
        }
        else if (quote && c == (unsigned char)quote)
        {
            quote = 0;
            p++;
        }
        else if (!quote && (c == '\'' || c == '"'))
        {
            quote = c;
            p++;
        }
        else
        {
            word[len++] = *p++;
        }
    }
    if (quote)
    {
        free(word);
        return -1;
    }
    if (!started)
    {
        free(word);
        return 0;
    }
    word[len] = '\0';
    *cursor = p;
    *result = word;
    return 1;
}

static int parse_command(const char *line, Command *cmd)
{
    const char *p = line;
    while (*p)
    {
        while (isspace((unsigned char)*p))
            p++;
        if (!*p)
            break;
        if (*p == '<' || *p == '>')
        {
            char op = *p++;
            char *filename = NULL;
            while (isspace((unsigned char)*p))
                p++;
            if (read_word(&p, &filename) != 1)
                return -1;
            char **target = op == '<' ? &cmd->input : &cmd->output;
            if (*target)
            {
                free(filename);
                return -1;
            }
            *target = filename;
        }
        else
        {
            char *word = NULL;
            if (read_word(&p, &word) != 1)
                return -1;
            if (cmd->count >= MAX_ARGS)
            {
                free(word);
                return -1;
            }
            cmd->args[cmd->count++] = word;
        }
    }
    cmd->args[cmd->count] = NULL;
    return 0;
}

static int redirect_file(const char *path, int target, int flags, mode_t mode)
{
    int fd = open(path, flags, mode);
    if (fd < 0)
    {
        error_message(errno);
        return -1;
    }
    if (dup2(fd, target) < 0)
    {
        int saved = errno;
        close(fd);
        error_message(saved);
        return -1;
    }
    close(fd);
    return 0;
}

static void execute_command(Command *cmd)
{
    if (cmd->count == 0)
        return;
    if (strcmp(cmd->args[0], "exit") == 0)
        return;
    if (strcmp(cmd->args[0], "cd") == 0)
    {
        const char *dir = cmd->count > 1 ? cmd->args[1] : getenv("HOME");
        if (!dir)
            dir = "/";
        if (chdir(dir) != 0)
            error_message(errno);
        return;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        error_message(errno);
        return;
    }
    if (pid == 0)
    {
        if (cmd->input && redirect_file(cmd->input, STDIN_FILENO, O_RDONLY, 0) < 0)
            _exit(1);
        if (cmd->output && redirect_file(cmd->output, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC, 0666) < 0)
            _exit(1);
        execvp(cmd->args[0], cmd->args);
        error_message(errno);
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0)
    {
        if (errno == EINTR)
            continue;
        error_message(errno);
        break;
    }
}

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;
    for (;;)
    {
        char *cwd = getcwd(NULL, 0);
        if (!cwd)
        {
            error_message(errno);
            break;
        }
        printf("%s$ ", cwd);
        free(cwd);
        fflush(stdout);
        if (getline(&line, &capacity, stdin) == -1)
            break;
        Command cmd = {0};
        if (parse_command(line, &cmd) != 0)
        {
            fprintf(stderr, "Error 22 (Invalid argument)\n");
            clear_command(&cmd);
            continue;
        }
        if (cmd.count && strcmp(cmd.args[0], "exit") == 0)
        {
            clear_command(&cmd);
            break;
        }
        execute_command(&cmd);
        clear_command(&cmd);
    }
    free(line);
    return 0;
}

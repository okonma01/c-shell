#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>

#include "myshell.h"

struct sigaction sa;
char cmd[100], *par[20], *p, *ptr, *funcs[100], *fun, *p_par[100][100];
int bg, infd[2], outfd[2], id, interrupt_happened;
pid_t curr_fg_pid, p_pid, c_pid, pipe_ids[100];

#define P_READ 0
#define P_WRITE 1

void pipe_chain(int *, int *, int);
void read_pipe(int *);
void write_pipe(int *);
void run_helper(char *);
void pipe_helper(char *, int);

void sigint_handler(int sig)
{
    // if there are any background processes, send kill system call with pid here
}

/*
 * Initializes the shell process, in particular its signal handling,
 * so that when an keyboard interrupt signal (Ctrl-C) is received by
 * the shell, it is instead directed to the process currently running
 * in the foreground.
 */
void initialize_signal_handling(void)
{

    /* TO BE COMPLETED BY THE STUDENT */

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

}

/*
 * Checks if any background processes have finished, without blocking
 * to wait for them. If any processes finished, prints a message for
 * each finished process containing the PID of the process.
 */
void print_finished_background_processes(void)
{

    /* TO BE COMPLETED BY THE STUDENT */

    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("Background process %d terminated\n", pid);
    }

}

/*
 * Reads a command line from standard input into a line buffer. If a
 * command-line could be read successfully (even if empty), returns a
 * positive value. If the reading process is interrupted by a keyboard
 * interrupt signal (Ctrl-C), returns 0. If a line cannot be read for
 * any other reason, including if an EOF (end-of-file) is detected in
 * standard input, exits the program (using the `exit` system call).
 *
 * The buffer is expected to be at least COMMAND_LINE_MAX_SIZE bytes
 * long. If the command line is longer than supported by this buffer
 * size (including line-feed character and null termination byte),
 * then ignores the full line and returns 0.
 *
 * If the command-line contains a comment character, any text
 * following the comment character is stripped from the command-line.
 *
 * Parameters:
 *  - line: pointer pointing to the start of the line buffer.
 * Returns:
 *  - 1 if a line could be read successfully, even if empty. 0 if the
 *    line could not be read due to a keyboard interrupt or for being
 *    too long.
 */
int read_command_line(char *line)
{

    /* TO BE COMPLETED BY THE STUDENT */

    if (fgets(line, COMMAND_LINE_MAX_SIZE + 1, stdin) == NULL)
    {
        if (feof(stdin))
        {
            printf("\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (strlen(line) > COMMAND_LINE_MAX_SIZE)
        {
            printf("Command line too long. Ignoring command.\n");
            return 0;
        }
        else
        {
            return 1;
        }
    }

}

/*
 * Executes the command (or commands) listed in the specified
 * command-line, according to the rules in the assignment
 * description. The line may contain no command at all, or may contain
 * multiple commands separated by operators like '&', ';' or '|'.
 *
 * Parameters:
 *  - line: string corresponding to the command line to be
 *    executed. This is typically the same line parsed by
 *    read_command_line, though for testing purposes you must assume
 *    that this can be any string containing commands in an
 *    appropriate format.
 */
void run_command_line(char *line)
{

    /* TO BE COMPLETED BY THE STUDENT */

    int i = 0;
    char *temp = strdup(line);
    char *token = strtok(temp, ";&|");
    while (token != NULL)
    {
        funcs[i] = token;
        token = strtok(NULL, ";&|");
        i++;
    }
    funcs[i] = NULL;
    for (int j = 0; j < i; j++)
    {
        pipe_helper(funcs[j], j);
    }
    free(temp);
}

void run_helper(char *line)
{
    int i = 0;
    char *temp = strdup(line);
    char *token = strtok(temp, " \n");
    while (token != NULL)
    {
        par[i] = token;
        token = strtok(NULL, " \n");
        i++;
    }
    par[i] = NULL;
    if (strcmp(par[0], "cd") == 0)
    {
        if (par[1] == NULL)
        {
            chdir(getenv("HOME"));
        }
        else
        {
            if (chdir(par[1]) == -1)
            {
                perror("chdir");
            }
        }
    }
    else if (strcmp(par[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else
    {
        c_pid = fork();
        if (c_pid == 0)
        {
            execvp(par[0], par);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else
        {
            if (bg)
            {
                printf("Background process started with PID %d\n", c_pid);
            }
            else
            {
                curr_fg_pid = c_pid;
                waitpid(c_pid, NULL, 0);
            }
        }
    }
    free(temp);
}

void read_pipe(int *fd) // PIPE READ
{

}

void write_pipe(int *fd) // PIPE WRITE
{
}

void pipe_helper(char *line, int i)
{
    if (strchr(line, '|'))
    {
        pipe_chain(infd, outfd, i);
    }
    else
    {
        run_helper(line);
    }
}

void pipe_chain(int *infd, int *outfd, int i)
{
    if (i)
    {
        pipe(infd);
    }
    if (i < 2)
    {
        pipe(outfd);
    }
    c_pid = fork();
    if (c_pid == 0)
    {
        if (i)
        {
            dup2(infd[P_READ], STDIN_FILENO);
            close(infd[P_READ]);
            close(infd[P_WRITE]);
        }
        if (i < 2)
        {
            dup2(outfd[P_WRITE], STDOUT_FILENO);
            close(outfd[P_READ]);
            close(outfd[P_WRITE]);
        }
        run_helper(funcs[i]);
    }
    else
    {
        if (i)
        {
            close(infd[P_READ]);
            close(infd[P_WRITE]);
        }
        if (i < 2)
        {
            close(outfd[P_READ]);
            close(outfd[P_WRITE]);
        }
        pipe_ids[i] = c_pid;
    }
}
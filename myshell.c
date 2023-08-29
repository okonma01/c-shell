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
  if (curr_fg_pid)
  {
    kill(curr_fg_pid, SIGINT);
  }
  interrupt_happened = 1;
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
  bg = interrupt_happened = 0;
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1)
  {
    perror("sigaction");
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
  int pid = waitpid(-1, NULL, WNOHANG);
  while (pid > 0)
  {
    printf("Background process [%d] finished.\n", pid);
    pid = waitpid(-1, NULL, WNOHANG);
  }
  // curr_fg_pid = 0;
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
      exit(0);
    }
    else if (ferror(stdin))
    {
      printf("\n");
      return 0;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    if ((int)strlen(line) >= 512)
    {
      printf("Command line too long.\n");
      return 0;
    }

    int ch = '#';
    // char *p;
    p = strchr(line, ch);
    if (p != NULL)
    {
      int pos = p - line;
      line[pos] = '\0';
    }
    // printf("%s\n", line);
    return 1;
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

  if (!strcmp(line, "exit\n") || !strcmp(line, "quit\n"))
  {
    exit(EXIT_SUCCESS);
  }

  if (!strchr(line, '|'))
  {
    p = strtok(line, " \n\t");
    if (p == NULL)
      return;
  }
  else
  {
    // PIPES HERE
    int k = 0;
    // printf("%s\n", line);
    fun = strtok(line, "|");

    while (fun != NULL)
    {
      // printf("%s\n", line);
      funcs[k] = fun;
      fun = strtok(NULL, "|");
      k++;
    }
    // exit(1);
    int i = 0;
    for (i = 0; i < k; i++)
    {
      int j = 0;
      char *p = strtok(funcs[i], " \n\t");

      while (p != NULL)
      {
        p_par[i][j] = p;
        p = strtok(NULL, " \n\t");
        j++;
      }
      p_par[i][j] = NULL;
    }

    for (i = 0; i < k; i++)
    {
      pipe_chain(i ? infd : NULL, i < k - 1 ? pipe(outfd), outfd : NULL, i);
      if (i)
      {
        close(infd[P_READ]);
      }
      if (i < k - 1)
      {
        infd[P_READ] = outfd[P_READ],
        close(outfd[P_WRITE]); // shell is done with it
      }
    }
    int t;
    for (t = 0; t < id; t++)
    {
      waitpid(pipe_ids[t], NULL, 0);
    }
    return;

    // PIPES END
  }

  ptr = strpbrk(line, ";&");
  if (ptr == NULL)
  {
    // printf("ptr null\n");
    run_helper(line);
    bg = 0;
    return;
  }
  while (ptr)
  {
    if (*ptr == '&')
    {
      bg = 1;
    }
    else
    {
      bg = 0;
    }
    *ptr = '\0';
    // bg = 1;
    run_helper(line);
    if (interrupt_happened)
      return;
    int pos = (ptr - line) + 1;
    line = &line[pos];
    ptr = strpbrk(line, ";&");
  }

  run_helper(line);
}

void run_helper(char *line)
{

  int i = 0;
  while (p)
  {
    par[i++] = strdup(p);
    p = strtok(NULL, " \n\t");
  }
  par[i] = NULL;

  if (*par[i - 1] == (char)'&')
  {
    // printf("here\n");
    par[i - 1] = NULL;
    bg = 1;
  }

  int rc = fork();
  if (rc < 0)
  { // fork failed; exit
    fprintf(stderr, "fork failed\n");
    exit(1);
  }
  else if (rc == 0)
  { // child (new process)
    // printf("hello, I am child (pid:%d)\n", (int)getpid());
    // printf("line: %s\n", line);
    // printf("par: %s\n", *par);

    int err = execvp(par[0], par);
    if (err == -1)
    {
      perror(par[0]);
    }
    exit(1);
  }
  else
  { // parent goes down this path (main)

    if (bg)
    {
      printf("Background process [%d] started.\n", rc);
      // bg = 0;
    }
    else
    {
      // set pid to rc here
      curr_fg_pid = rc;
      while (waitpid(rc, NULL, 0) != curr_fg_pid)
      {
        // printf("he");
        // sleep(5);
        /* code */
      }

      // waitpid(rc, NULL, 0);
      // return;
    }
  }
}

void read_pipe(int *fd) // PIPE READ
{
  dup2(fd[P_READ], STDIN_FILENO);
  close(fd[P_READ]);
  // close(fd[P_WRITE]);
}

void write_pipe(int *fd) // PIPE WRITE
{
  dup2(fd[P_WRITE], STDOUT_FILENO);
  close(fd[P_READ]);
  close(fd[P_WRITE]);
}

void pipe_helper(char *line, int i)
{
  p_pid = fork();

  if (p_pid < 0)
  { // fork failed; exit
    fprintf(stderr, "fork failed\n");
    exit(1);
  }
  else if (p_pid == 0)
  {
    if (infd)
    {
      read_pipe(infd);
    }
    if (outfd)
    {
      write_pipe(outfd);
    }

    int err = execvp(p_par[i][0], p_par[i]);
    if (err == -1)
    {
      perror(p_par[i][0]);
    }
    exit(1);
  }
  else
  {
    if (bg)
    {
      printf("Background process [%d] started.\n", p_pid);
    }
    else
    {
      // set pid to p_pid here
      curr_fg_pid = p_pid;
      pipe_ids[id++] = p_pid;
      // pid_t corpse = waitpid(-1, NULL, 0);
      // while (corpse != -1)
      // {
      // }
    }
    // c_pid = waitpid(-1, NULL, 0);
  }
}

void pipe_chain(int *infd, int *outfd, int i)
{

  ptr = strpbrk(*p_par[i], ";&");
  if (ptr == NULL)
  {
    // printf("ptr null\n");
    pipe_helper(*p_par[i], i);
    bg = 0;
    return;
  }
  while (ptr)
  {
    if (*ptr == '&')
    {
      bg = 1;
      *ptr = '\0';
    }
    else
    {
      bg = 0;
    }
    *ptr = '\0';
    // bg = 1;
    pipe_helper(*p_par[i], i);
    if (interrupt_happened)
      return;
    int pos = (ptr - *p_par[i]) + 1;
    *p_par[i] = p_par[i][pos];
    ptr = strpbrk(*p_par[i], ";&");
  }

  pipe_helper(*p_par[i], i);
}
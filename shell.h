//
// Created by asusrog on 14/06/2021.
//

#ifndef UNTITLED_SHELL_H
#define UNTITLED_SHELL_H


#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define SHELL_RL_BUFFSIZE 1024
#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

#include <sys/wait.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "util.h"


/*
   ----------------- Shell ---------------
 */

pid_t shell_pgid;
int shell_terminal;
struct termios shell_tmodes;
int shell_is_interactive;

void init_shell();
void launch_process (process *p, pid_t pgid, int infile, int outfile, int errfile,
                     int foreground);
void launch_job(job *j, int foreground);
void shell_loop(void);
char *shell_readLine(void);
char **shell_split_line(char *line);
int shell_launch(char **args);
int shell_execute(char **args);
void shell_processTokens(job *j, char **args);

// Commands
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

int shell_num_builtins();


#endif //UNTITLED_SHELL_H

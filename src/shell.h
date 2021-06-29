//
// Created by houssem on 29/06/2021.
//

#ifndef MICROSHELL_shell_H
#define MICROSHELL_shell_H



#include "headers.h"
#include "process.h"
#include "job.h"

#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define SHELL_RL_BUFFSIZE 1024
#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"



/*
   ----------------- Shell ---------------
 */



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


#endif //MICROSHELL_shell_H

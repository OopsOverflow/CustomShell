//
// Created by houssem on 29/06/2021.
//

#ifndef MICROSHELL_shell_H
#define MICROSHELL_shell_H



#include "../util/headers.h"
#include "../process/process.h"
#include "../job/job.h"

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

// Alpha fx
/**
 * Copies a file and gives same access rights to its resulting copy
 * @param input file to be copied
 * @param output fresh copy of input
 * @return
 */
void copyFileWithAccessRights(const char *input, const char *output);

/**
 * Creates a anew path from an already exsiting path and
 * a file.
 * @param path
 * @param file
 * @return new path
 */
char *makepath(const char *path, const char *file);

/**
 * Copies only files within a directory into the output directory
 * path.
 * @param inputDirectory the directory to be copied
 * @param outputDirectory target directory to be copied to
 */
void copydir(const char *inputDirectory, const char *outputDirectory);

/**
 * Copies all elements of a directory recursively.
 * i.e: including sub-folders.
 * @param inputDirectory the directory to be copied
 * @param outputDirectory target directory to be copied to
 */
void copyDirectoryRecusivly(const char *inputDirectory, const char *outputDirectory);


// Commands
int shell_cd(char **args);
int shell_help(char **args);
int shell_clear(char **args);
int shell_exit(char **args);
int shell_pwd(char **args);
int shell_cp(char **args);
int shell_mkdir(char **args);

int shell_num_builtins();


#endif //MICROSHELL_shell_H

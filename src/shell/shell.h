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

// Shell init functions
void init_shell();
void launch_process (process *p, pid_t pgid, int infile, int outfile, int errfile,
                     int foreground);
void launch_job(job *j, int foreground);
void shell_loop(void);
int shell_launch(char **args);
int shell_execute(char **args);
void shell_processTokens(job *j, char **args);

/*
 -------------Shell parser functions--------
*/
char *shell_readLine(void);
char **shell_split_line(char *line);
int shell_parse_chevron_in(char **pString);

int shell_parse_chevron_out(char **pString);


/*
 -------------Auxiliary Functions -----------
 */

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


/*
 ------------- Shell Commands -------------
 */
/**
 * Change directory
 * @param args path
 * @return
 */
int shell_cd(char **args);
/**
 * prints help
 * @param args
 * @return
 */
int shell_help(char **args);
/**
 * clear screen
 * @param args
 * @return
 */
int shell_clear(char **args);
/**
 * exits shell
 * @param args
 * @return
 */
int shell_exit(char **args);
/**
 * prints current working directory
 * @param args
 * @return
 */
int shell_pwd(char **args);
/**
 * shell copy function for files and directories
 * @param args tokens and targets
 * @return
 */
int shell_cp(char **args);
/**
 * Makes a list of directories from args if they dont
 * already exist
 * @param args dirs to make
 * @return
 */
int shell_mkdir(char **args);

/**
 * Prints shell builtin functions
 * @return
 */
int shell_num_builtins();


#endif //MICROSHELL_shell_H

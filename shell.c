//
// Created by asusrog on 14/06/2021.
//
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

char *builtin_str[] = {
        "cd",
        "help",
        "exit"
};

int (*builtin_func[]) (char **) = {
        &shell_cd,
        &shell_help,
        &shell_exit
};

int main(int argc, char **argv)
{
    // Config Files
    // Command Line Loop
    init_shell();
    shell_loop();
    // Shut Down / Clean Up
    return EXIT_SUCCESS;
}

void init_shell() {
    /* See if we are running interactively.  */
    shell_terminal = STDIN_FILENO;
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        /* Loop until we are in the foreground.  */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        /* Ignore interactive and job-control signals.  */
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);

        /* Put ourselves in our own process group.  */
        shell_pgid = getpid();
        if (setpgid(shell_pgid, shell_pgid) < 0) {
            perror("Couldn't put the shell in its own process group");
            exit(1);
        }

        /* Grab control of the terminal.  */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save default terminal attributes for shell.  */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

void launch_process(process *p, pid_t pgid,
                           int infile, int outfile, int errfile,
                           int foreground) {
    pid_t pid;

    if (shell_is_interactive) {
        /* Put the process into the process group and give the process group
           the terminal, if appropriate.
           This has to be done both by the shell and in the individual
           child processes because of potential race conditions.  */
        pid = getpid();
        if (pgid == 0) pgid = pid;
        setpgid(pid, pgid);
        if (foreground)
            tcsetpgrp(shell_terminal, pgid);

        /* Set the handling for job control signals back to the default.  */
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
    }

    /* Set the standard input/output channels of the new process.  */
    if (infile != STDIN_FILENO) {
        dup2(infile, STDIN_FILENO);
        close(infile);
    }
    if (outfile != STDOUT_FILENO) {
        dup2(outfile, STDOUT_FILENO);
        close(outfile);
    }
    if (errfile != STDERR_FILENO) {
        dup2(errfile, STDERR_FILENO);
        close(errfile);
    }

    /* Exec the new process.  Make sure we exit.  */
    execvp(p->argv[0], p->argv);
    perror("execvp");
    exit(1);
}

void launch_job(job *j, int foreground) {
    process *p;
    pid_t pid;
    int mypipe[2], infile, outfile;

    infile = j->stdin;
    for (p = j->first_process; p; p = p->next) {
        /* Set up pipes, if necessary.  */
        if (p->next) {
            if (pipe(mypipe) < 0) {
                perror("pipe");
                exit(1);
            }
            outfile = mypipe[1];
        } else
            outfile = j->stdout;

        /* Fork the child processes.  */
        pid = fork();
        if (pid == 0)
            /* This is the child process.  */
            launch_process(p, j->pgid, infile,
                           outfile, j->stderr, foreground);
        else if (pid < 0) {
            /* The fork failed.  */
            perror("fork");
            exit(1);
        } else {
            /* This is the parent process.  */
            p->pid = pid;
            if (shell_is_interactive) {
                if (!j->pgid)
                    j->pgid = pid;
                setpgid(pid, j->pgid);
            }
        }

        /* Clean up after pipes.  */
        if (infile != j->stdin)
            close(infile);
        if (outfile != j->stdout)
            close(outfile);
        infile = mypipe[0];
    }

    format_job_info(j, "launched");

    if (!shell_is_interactive)
        wait_for_job(j);
    else if (foreground)
        put_job_in_foreground(j, 0);
    else
        put_job_in_background(j, 0);
}


void shell_loop(void)
{
    char *line;
    char **args;
    int status;
    do
    {
        printf("【ツ】 ");
        line = shell_readLine();
        args = shell_split_line(line);
        job *theJob = malloc(sizeof(job));
        shell_processTokens(theJob, args);
        //launch_job(theJob);
        //status = shell_execute(args);
        free(line);
        free(args);
    } while (status);
}

void shell_processTokens(job *j, char ** args) {
    int i;
    for(i = 0; i < sizeof(args) / sizeof(args[0]); i=i+1) {
            if(strcmp(args[i],"|") == 0) {
                printf(" Pipe !\n");
                break;
            }
            else
                printf("Invalid input\n" );
    }
}

/*
 *  The code in this method could be replaced with the following:
 *
 *      char *line = NULL;
 *      ssize_t bufsize = 0; // have getline allocate a buffer for us
 *      getline(&line, &bufsize, stdin);
 *      return line;
 *
 *  Keeping it this way for more control of buffer reallocation later on
 */
char *shell_readLine(void)
{
    int buffsize = SHELL_RL_BUFFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffsize);
    int c;
    if(!buffer) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while(1) {
        // Read a character
        c = getchar();
        // If EOF, replace with null and return
        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        // Check if we have exceeded the buffer and reallocate if necessary
        if(position >= buffsize) {
            buffsize += SHELL_RL_BUFFSIZE;
            buffer = realloc(buffer, buffsize);
            if(!buffer) {
                fprintf(stderr, "shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*
 *  Does not allow quoting or backslash escaping in command line args
 */
char **shell_split_line(char *line) {
    int buffsize = SHELL_TOK_BUFFSIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char *));
    char *token;
    if(!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, SHELL_TOK_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;
        if(position >= buffsize) {
            buffsize += SHELL_TOK_BUFFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char *));
            if(!tokens) {
                fprintf(stderr, "shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int shell_launch(char **args) {
    pid_t pid, wpid;
    int status;
    pid=fork();
    if(pid==0) {
        if(execvp(args[0],args)==-1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        perror("shell");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int shell_execute(char **args) {
    int i;
    if(args[0] == NULL) {
        return 1;
    }
    for(i=0; i<shell_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i]) (args);
        }
    }
    return shell_launch(args);
}

int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "shell: expected arg to \"cd\"\n");
    } else {
        if(chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char **args) {
    int i;
    printf("\nThis is a shell.\n\n");
    printf("The following commands are built in:\n");
    for(i=0;i<shell_num_builtins();i++) {
        printf("    %s\n",builtin_str[i]);
    }
    printf("\na man is a dogs best friend\n");
    printf("...good day\n\n");
    return 1;
}

int shell_exit(char **args) {
    return 0;
}
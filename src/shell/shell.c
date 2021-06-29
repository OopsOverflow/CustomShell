//
// Created by asusrog on 14/06/2021.
//
#include <fcntl.h>
#include <pwd.h>

#include "shell.h"
#include "../util/colors.h"


/*typedef enum
{
    TOKEN_REDIRECTION,
    TOKEN_PIPE,

};
typedef struct
{
    char content[128];
    u_int32_t size;
    e_token_type type;
} t_lexer_token;

typedef struct
{
    t_lexer_token tokens[2048];
    u_int32_t size;
} t_lexer;

static const t_oplist existing_token[] ={
        {">", 1, TOKEN_REDIRECTION},
        {">&", 2, TOKEN_REDIRECTION},
        {"<", 1, TOKEN_REDIRECTION},
        {"<&", 2, TOKEN_REDIRECTION},
        {"<>", 2, TOKEN_REDIRECTION},
        {">>", 2, TOKEN_REDIRECTION},
        {"<<", 2, TOKEN_REDIRECTION},
        {">|", 2, TOKEN_REDIRECTION},
        {">>-", 3, TOKEN_REDIRECTION},
        {">|", 2, TOKEN_REDIRECTION},
        {"|", 1, TOKEN_PIPE},
}; */



void shell_parse(char **args, char *line);

void init_shell() {
    pid_t shell_pgid;
    int shell_terminal;
    struct termios shell_tmodes;
    int shell_is_interactive;
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

    pid_t shell_pgid;
    int shell_terminal;
    struct termios shell_tmodes;
    int shell_is_interactive;

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
    int shell_is_interactive;

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

void shell_display(){
    char hostname[65536];
    int host = gethostname(hostname, sizeof(hostname));
    if (host){
        perror("Error displaying the host name");
        exit(EXIT_FAILURE);
    }
    uid_t uid = getuid();
    struct passwd *passwd = getpwuid(uid);
    if(!passwd){
        perror("Error printing the username");
        exit(EXIT_FAILURE);
    }
    char *username = NULL;
    // Affiche le chemin du working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        username = getenv("USER");
        printf(ANSI_BLUE_BG);
        printf(ANSI_WHITE);
        printf("%s",username);
        printf(ANSI_RESET);
        printf("@");
        printf(ANSI_RESET);
        printf(ANSI_GREY_BG);
        printf(ANSI_BLACK);
        printf("%s",cwd);
        printf(ANSI_RESET);
        printf(ANSI_BLUE_BG);
        printf(ANSI_WHITE);
        printf("$");
        printf(ANSI_RESET);
        printf(" ");
    } else {
        perror("getcwd() error");
    }

}

void shell_loop(void)
{
    char* descr = "           _.---._\n       ./.'"".'/|\\`.""'.\\.        MicroShell\n      :  .' / | \\ `.  :       Made By:\n      '.'  /  |  \\  `.'       Esteban & Houssem\n       `. /   |   \\ .'\n         `-.__|__.-'\n\n";
    printf("%s",descr);
    char *line;
    char **args;
    int status;
    do
    {
        shell_display();
        // Execution de la fonction shell_readLine
        line = shell_readLine();
        args = shell_split_line(line);
        shell_parse(args, line); // test sur le parsage pour gérer les chevrons
        job *theJob = malloc(sizeof(job));
        shell_processTokens(theJob, args);
        //launch_job(theJob);
        status = shell_execute(args);
        free(line);
        free(args);
    } while (status);
}

void shell_parse(char **args, char *line) {
    for(char **arg = args; *arg; ++arg){
        printf("test : %s\n", *arg);
        if(strcmp(*arg, ">") == 0){
            // Faire quelque chose avec la redirection >
            // On essaie de reconnaître le filename
            char ** c = ++arg;
            printf("filename : %s\n", *c);
            int fDesc = open(*c, O_WRONLY, 0666);
            printf("Value du fileDescriptor de l'open du fichier : %i\n", fDesc);
        }
        else if(strcmp(*arg, "<") == 0){
            // Faire quelque chose avec la redirection <
        }
    }
}

void shell_processTokens(job *j, char ** args) {
    int i;
    for(i = 0; i < sizeof(args) / sizeof(args[0]); i=i+1) {
        if(strcmp(args[i],"|") == 0) {
            printf(" Pipe !\n");
            break;
        }
        else
            //printf("Invalid input\n" );
            continue;
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




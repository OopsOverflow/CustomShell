//
// Created by asusrog on 14/06/2021.
//
#include <fcntl.h>
#include <pwd.h>

#include "shell.h"
#include "../util/colors.h"


/**
 * Initialize shell by configuring its properties
 */
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

/**
 * 
 * @param p 
 * @param pgid 
 * @param infile 
 * @param outfile 
 * @param errfile 
 * @param foreground 
 */
void launch_process(process *p, pid_t pgid,
                    int infile, int outfile, int errfile,
                    int foreground) {
    pid_t pid;
    int shell_terminal = STDIN_FILENO;
    int shell_is_interactive = isatty(shell_terminal);

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

    int shell_terminal = STDIN_FILENO;
    int shell_is_interactive = isatty(shell_terminal);

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
/**
 * Controls the shells looks and text coloring
 */
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
/*
 * Parser function that allows for parsing ">" commands
 */
int shell_parse_chevron_out(char **args) {
    for (char **arg = args ; *arg ; ++arg){
        if(strcmp(*arg, ">") == 0){
            char *f = *(arg+1);
            if(f==NULL){
                perror("[ERROR] Pas de fichier pour la redirection >");
                return -1;
            }
            int fDesc = open(f, O_CREAT | O_WRONLY | O_TRUNC, 0644);

            if(fDesc == -1){
                perror("[ERROR] Fichier invalide");
                return -1;
            }
            return fDesc;
        }
    }
    return STDIN_FILENO;
}

/*
 * Parser function that allows for parsing "<" commands
 */
int shell_parse_chevron_in(char **args) {
    for (char **arg = args ; *arg ; ++arg){
        if(strcmp(*arg, "<") == 0){
            char *f = *(arg+1);
            if(f==NULL){
                perror("[ERROR] Pas de fichier après la redirection <");
                return -1;
            }
            int fDesc = open(f, O_RDONLY, 0644);

            if(fDesc == -1){
                perror("[ERROR] Fichier invalide");
                return -1;
            }
            return fDesc;
        }
    }
    return STDIN_FILENO;
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

/**
 * Parser function to split line into different arguments
 * @param line parsed line
 * @return list of arguments
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

/**
 * parser function to split cstring using chars
 * @param list of args to split
 * @param c the characters used to split
 * @return split commands
 */
static char **split(char *args, char *c){
    char *ptr = NULL;
    char **cmd_n = NULL;
    size_t idx = 0;
    ptr = strtok(args, c);
    while(ptr){
        if(strlen(ptr) > 0){
            cmd_n = (char **) realloc(cmd_n, (cmd_n, (idx+1) * sizeof(char *)));
            cmd_n[idx] = strdup(ptr);
        }
        ptr = strtok(NULL, c);
        ++idx;
    }

    if(idx>0){
        if(cmd_n[idx-1] != NULL){
            int lenghtStr = strlen(cmd_n[idx-1]);
            if (cmd_n[idx-1][lenghtStr-1] == '\n'){
                cmd_n[idx-1][lenghtStr-1] = '\0';
            }
        }
    }
    cmd_n = (char **) realloc(cmd_n, ((idx+1) * sizeof(char *)));
    cmd_n[idx] = NULL;

    return (cmd_n);
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
        //shell_parse(args, line); // test sur le parsage pour gérer les chevrons
        job *theJob = malloc(sizeof(job));
        int in = shell_parse_chevron_in(args);
        int out = shell_parse_chevron_out(args);
        theJob->stdin = in;
        theJob->stdout = out;
        theJob->stderr = STDERR_FILENO;
        //shell_processTokens(theJob, args);
        //launch_job(theJob, 0);

        // Si il y a un/des pipe(s), on sépare la commande en plusieurs commande grâce à la fonction split
        char **cmds = split(line, "|");

        for(char **str = cmds; *str; str++){
            char **splittedCmd = split(*str, " ");

            process *p = (process *) malloc(sizeof(process));
            p->argv = splittedCmd;
            p->next = NULL;

            for(char **arg = splittedCmd; *arg; ++arg){
                if(strcmp(*arg, ">") == 0){
                    *arg = NULL;
                }
                else if(strcmp(*arg, "<") == 0) {
                    *arg = NULL;
                }
            }

            if(theJob->first_process){
                process *last_proc = theJob->first_process;
                while(last_proc->next){
                    last_proc = last_proc->next;
                }
                last_proc->next = p;
            }
            else{
                theJob->first_process = p;
            }
        }
        launch_job(theJob,1);

        status = shell_execute(args);
        free(line);
        free(args);
    } while (status);
}



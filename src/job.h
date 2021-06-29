//
// Created by houssem on 29/06/2021.
//

#ifndef MICROSHELL_job_H
#define MICROSHELL_job_H

#include "headers.h"
#include "process.h"

/* A job is a pipeline of processes.  */
typedef struct job
{
    struct job *next;           /* next active job */
    char *command;              /* command line, used for messages */
    process *first_process;     /* list of processes in this job */
    pid_t pgid;                 /* process group ID */
    char notified;              /* true if user told about stopped job */
    struct termios tmodes;      /* saved terminal modes */
    int stdin, stdout, stderr;  /* standard i/o channels */
} job;


/* Find the active job with the indicated pgid.  */
job *find_job (pid_t pgid);

/* Return true if all processes in the job have stopped or completed.  */
int job_is_stopped (job *j);

/* Return true if all processes in the job have completed.  */
int job_is_completed (job *j);

/* Put job j in the foreground.  If cont is nonzero,
   restore the saved terminal modes and send the process group a
   SIGCONT signal to wake it up before we block.  */
void put_job_in_foreground (job *j, int cont);

/* Put a job in the background.  If the cont argument is true, send
   the process group a SIGCONT signal to wake it up.  */
void put_job_in_background (job *j, int cont);

/* Store the status of the process pid that was returned by waitpid.
   Return 0 if all went well, nonzero otherwise.  */
int mark_process_status (pid_t pid, int status);

/* Check for processes that have status information available,
   without blocking.  */
void update_status (void);

/* Check for processes that have status information available,
   blocking until all processes in the given job have reported.  */
void wait_for_job (job *j);

/* Format information about job status for the user to look at.  */
void format_job_info (job *j, const char *status);

/* Notify the user about stopped or terminated jobs.
   Delete terminated jobs from the active job list.  */
void do_job_notification ();



#endif //MICROSHELL_job_H

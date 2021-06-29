//
// Created by houssem on 29/06/2021.
//

#ifndef MICROSHELL_process_H
#define MICROSHELL_process_H

#include "../util/headers.h"
#include "../job/job.h"

/* A process is a single process.  */
typedef struct process
{
    struct process *next;       /* next process in pipeline */
    char **argv;                /* for exec */
    pid_t pid;                  /* process ID */
    char completed;             /* true if process has completed */
    char stopped;               /* true if process has stopped */
    int status;                 /* reported status value */
} process;


/* Store the status of the process pid that was returned by waitpid.
   Return 0 if all went well, nonzero otherwise.  */
int mark_process_status (pid_t pid, int status);

/* Check for processes that have status information available,
   without blocking.  */
void update_status (void);

/* Check for processes that have status information available,
   blocking until all processes in the given job have reported.  */
void wait_for_job (job *j);


#endif //MICROSHELL_process_H

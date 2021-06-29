//
// Created by houssem on 29/06/2021.
//

#include "process.h"
#include "../job/job.h"


/* Store the status of the process pid that was returned by waitpid.
   Return 0 if all went well, nonzero otherwise.  */
int mark_process_status (pid_t pid, int status) {
    job* first_job;
    job *j;
    process *p;
    if (pid > 0) {
        /* Update the record for the process.  */
        for (j = first_job; j; j = j->next)
            for (p = j->first_process; p; p = p->next)
                if (p->pid == pid) {
                    p->status = status;
                    if (WIFSTOPPED (status))
                        p->stopped = 1;
                    else {
                        p->completed = 1;
                        if (WIFSIGNALED (status))
                            fprintf (stderr, "%d: Terminated by signal %d.\n", (int) pid, WTERMSIG (p->status));
                    }
                    return 0;
                }
        fprintf (stderr, "No child process %d.\n", pid);
        return -1;
    }
    else if (pid == 0 || errno == ECHILD)
        /* No processes ready to report.  */
        return -1;
    else {
        /* Other weird errors.  */
        perror ("waitpid");
        return -1;
    }
}

/* Check for processes that have status information available,
   without blocking.  */
void update_status (void) {
    int status;
    pid_t pid;

    do
        pid = waitpid (WAIT_ANY, &status, WUNTRACED|WNOHANG);
    while (!mark_process_status (pid, status));
}

/* Check for processes that have status information available,
   blocking until all processes in the given job have reported.  */
void wait_for_job (job *j) {
    int status;
    pid_t pid;

    do
        pid = waitpid (WAIT_ANY, &status, WUNTRACED);
    while (!mark_process_status (pid, status) && !job_is_stopped (j) && !job_is_completed (j));
}


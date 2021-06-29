//
// Created by houssem on 29/06/2021.
//

#include "job.h"
#include "process.h"
#include "shell.h"

/* The active jobs are linked into a list.  This is its head.   */
job *first_job;

/* Find the active job with the indicated pgid.  */
job *find_job (pid_t pgid) {

    job *j;

    for (j = first_job; j; j = j->next)
        if (j->pgid == pgid)
            return j;

    return NULL;
}

/* Return true if all processes in the job have stopped or completed.  */
int job_is_stopped (job *j) {

    process *p;

    for (p = j->first_process; p; p = p->next)
        if (!p->completed && !p->stopped)
            return 0;

    return 1;
}

/* Return true if all processes in the job have completed.  */
int job_is_completed (job *j) {

    process *p;

    for (p = j->first_process; p; p = p->next)
        if (!p->completed)
            return 0;

    return 1;
}

/* Put job j in the foreground.  If cont is nonzero,
   restore the saved terminal modes and send the process group a
   SIGCONT signal to wake it up before we block.  */

void put_job_in_foreground (job *j, int cont) {
    pid_t shell_pgid;
    int shell_terminal;
    /* Put the job into the foreground.  */
    tcsetpgrp (shell_terminal, j->pgid);
    /* Send the job a continue signal, if necessary.  */
    if (cont) {
        tcsetattr (shell_terminal, TCSADRAIN, &j->tmodes);
        if (kill (- j->pgid, SIGCONT) < 0)
            perror ("kill (SIGCONT)");
    }
    /* Wait for it to report.  */
    wait_for_job (j);

    /* Put the shell back in the foreground.  */
    tcsetpgrp (shell_terminal, shell_pgid);

    /* Restore the shell’s terminal modes.  */
    tcgetattr (shell_terminal, &j->tmodes);
    tcsetattr (shell_terminal, TCSADRAIN, (const struct termios *) &shell_terminal);
}

/* Put a job in the background.  If the cont argument is true, send
   the process group a SIGCONT signal to wake it up.  */
void put_job_in_background (job *j, int cont) {
    /* Send the job a continue signal, if necessary.  */
    if (cont)
        if (kill (-j->pgid, SIGCONT) < 0)
            perror ("kill (SIGCONT)");
}

/* Format information about job status for the user to look at.  */
void format_job_info (job *j, const char *status) {
    fprintf (stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

/* Notify the user about stopped or terminated jobs.
   Delete terminated jobs from the active job list.  */
void do_job_notification (void) {
    job *j, *jlast, *jnext;
    process *p;

    /* Update status information for child processes.  */
    update_status ();

    jlast = NULL;
    for (j = first_job; j; j = jnext) {
        jnext = j->next;

        /* If all processes have completed, tell the user the job has
         completed and delete it from the list of active jobs.  */
        if (job_is_completed (j)) {
            format_job_info (j, "completed");

            if (jlast)
                jlast->next = jnext;
            else
                first_job = jnext;

            //free_job(j);
        }
            /* Notify the user about stopped jobs,
             marking them so that we won’t do this more than once.  */
        else if (job_is_stopped (j) && !j->notified) {
            format_job_info (j, "stopped");
            j->notified = 1;
            jlast = j;
        }
            /* Don’t say anything about jobs that are still running.  */
        else
            jlast = j;
    }
}


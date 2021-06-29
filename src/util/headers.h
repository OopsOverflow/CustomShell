//
// Created by houssem on 28/06/2021.
//

#include <sys/wait.h>
#include <termios.h>
#include <errno.h>
#include <asm-generic/errno.h>
#include <termios.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

typedef struct process process;

typedef struct job job;

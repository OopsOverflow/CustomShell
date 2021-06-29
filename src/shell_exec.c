//
// Created by houssem on 29/06/2021.
//
#include "shell/shell.h"

int main(int argc, char **argv)
{
    // Config Files
    // Command Line Loop
    init_shell();
    shell_loop();
    // Shut Down / Clean Up
    return EXIT_SUCCESS;
}
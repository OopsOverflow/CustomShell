//
// Created by houssem on 29/06/2021.
//

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shell.h"
#include "../util/colors.h"

char *builtin_str[] = {
        "cd",
        "help",
        "clear",
        "pwd",
        "cp",
        "mkdir",
        "exit"
};

int (*builtin_func[]) (char **) = {
        &shell_cd,
        &shell_help,
        &shell_clear,
        &shell_pwd,
        &shell_cp,
        &shell_mkdir,
        &shell_exit
};

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
            perror("[ERROR] cd");
        }
        /*else{
            printf(getcwd(, (size_t)size));
        }*/
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
int shell_pwd(char **args)
{
    char pwd[1024];

    getcwd(pwd, sizeof(pwd));
    printf(ANSI_RED);
    printf(">>%s\n", pwd);
    printf(ANSI_RESET);
    return 1;
}

int shell_clear(char **args)
{
    printf(ANSI_RESET_SCREEN);
    return 1;
}

int shell_exit(char **args) {
    return 0;
}

int shell_cp(char **args) {
    printf(args[0]);
    printf("\n");
    printf(args[1]);
    printf("\n");
    printf(args[2]);
    printf("\n");
    printf(args[3]);
    printf("\n");

    if (strcmp(args[1], "-r") && (!args[2] || !args[3])) {
        printf(ANSI_RED);
        printf("a shell expected arguments\n");
        printf(ANSI_RESET);
        return 1;
    }else if (!args[1]||!args[2]) {
        printf(ANSI_RED);
        printf("shell expected arguments\n");
        printf(ANSI_RESET);
        return 1;
    }


    struct stat istat;
    if (args[3]) {
        stat(args[2], &istat);
        // Check if directory or file.
        if (S_ISDIR(istat.st_mode)) {
            copyDirectoryRecusivly(args[2], args[3]);
        } else {
            printf("shell : flag error. Check cp --help for more info.\n");
        }
    } else {
        stat(args[1], &istat);
        if (S_ISDIR(istat.st_mode)) {
            copydir(args[1], args[2]);
        } else {
            copyFileWithAccessRights(args[1], args[2]);
        }
        return 1;
    }
}

char *makepath(const char *path, const char *file) {
    int lpath = strlen(path);
    int lfile = strlen(file);
    // Allocate dynamic memory
    char *res = malloc(lpath + lfile + 2);
    strcpy(res, path);
    // Add / for directory
    if (lpath != 0 && path[lpath - 1] != '/')
        strcat(res, "/");
    strcat(res, file);
    return res;
}

void copyFileWithAccessRights(const char *input, const char *output) {
    // Try to open input file in read only mode
    int idesc = open(input, O_RDONLY);
    if (idesc == -1) {
        perror("ERROR: Can't Open Input File");
        return;
    }
    // Try to open the output file
    // If it doesn't exist create it using the
    // O_CREAT flag
    int odesc = open(output, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (odesc == -1) {
        perror("ERROR: Can't Open Output File");
        return;
    }
    // Give new file the same access rights as the old one
    struct stat istat;
    if (fstat(idesc, &istat) < 0) {
        perror("ERROR: Cannot Change Access Rights");
        return;
    }
    if (fchmod(odesc, istat.st_mode) < 0) {
        perror("ERROR: Cannot Change Access Rights");
        return;
    }

    while (1) {
        // Copy file block by block
        char buffer[4096];
        int rcnt = read(idesc, buffer, sizeof(buffer));
        if (rcnt == 0)
            break; // End of file
        if (rcnt < 0) {
            // Restart Call
            if (errno == EAGAIN || errno == EINTR)
                continue;
            perror("ERROR: Can't Read File");
            return;
        }
        // Writing ...
        int pos = 0;
        while (rcnt != 0) {
            int wcnt = write(odesc, buffer + pos, rcnt);
            if (wcnt < 0) {
                // Until bloc finished
                if (errno == EAGAIN || errno == EINTR)
                    continue;
                perror("ERROR: Can't Write To File");
                return;
            }
            rcnt -= wcnt;
            pos += wcnt;
        }
    }
    // Close opened readers
    close(idesc);
    close(odesc);

    printf("File Copy: Done\n");
}

void copydir(const char *inputDirectory, const char *outputDirectory) {
    // Try to ope the input directory
    DIR *dir = opendir(inputDirectory);
    if (dir == NULL) {
        perror("ERROR: Invalid Input Directory");
        return;
    }

    // Give output directory same access rights as input
    struct stat istat;
    stat(inputDirectory, &istat);
    mkdir(outputDirectory, 0777);
    chmod(outputDirectory, istat.st_mode);

    while (1) {
        struct dirent *entry = readdir(dir);
        if (entry == NULL)
            break;
        if (entry->d_name[0] == '.')
            continue;
        // Create paths
        char *inputDirName = makepath(inputDirectory, entry->d_name);
        char *outputDirName = makepath(outputDirectory, entry->d_name);
        copyFileWithAccessRights(inputDirName, outputDirName);
        // Release dynamically allocated memory.
        free(inputDirName);
        free(outputDirName);
    }
    // Close opened directory
    closedir(dir);
}

void copyDirectoryRecusivly(const char *inputDirectory, const char *outputDirectory) {
    // Try to ope the input directory
    DIR *dir = opendir(inputDirectory);
    if (dir == NULL) {
        perror("ERROR: Invalid Input Directory");
        return;
    }

    // Give output directory same access rights as input
    struct stat istat;
    if (stat(inputDirectory, &istat) < 0) {
        perror("ERROR: Cannot Change Access Rights");
        return;
    }
    if (mkdir(outputDirectory, 0777) < 0) {
        perror("ERROR While Opening Output Directory");
        return;
    }
    if (chmod(outputDirectory, istat.st_mode) < 0) {
        perror("ERROR: Cannot Change Access Rights");
        return;
    }

    // Copy all contents of the input directory recursively
    while (1) {
        errno = 0; // To mark EOF
        struct dirent *ent = readdir(dir);
        if (ent == NULL) {
            if (errno == 0)
                break;
            perror("ERROR: Can't Read Directory Contents");
            return;
        }

        // Don't copy hidden files or ..
        if (ent->d_name[0] == '.')
            continue;

        // Set Names and create new paths
        char *inputDirName = makepath(inputDirectory, ent->d_name);
        char *outputDirName = makepath(outputDirectory, ent->d_name);

        struct stat istat;
        if (stat(inputDirName, &istat) < 0) {
            perror("ERROR: Cannot Change Access Rights");
            return;
        }
        if (S_ISDIR(istat.st_mode))
            // Copy recursively if another directory
            copyDirectoryRecusivly(inputDirName, outputDirName);
        else
            // Copy file if file
            copyFileWithAccessRights(inputDirName, outputDirName);

        free(inputDirName);
        free(outputDirName);
    }
    // Close directory
    closedir(dir);
    printf("--- Dir Copy : DONE\n");
}

int shell_mkdir(char **args) {
    // Test for not enough arguments
    if (!args[1]) {
        printf(ANSI_RED);
        printf("mkdir: not enough arguments\n");
        return 1;
    }

    // copy all given directories
    int i = 1;

    while (args[i]) {
        // See if directory already exits
        struct stat st = {0};

        if (stat(args[i], &st) == -1) {
            mkdir(args[i], 0700);
            i++;
        } else {
            printf(ANSI_BLUE);
            printf("ERROR: Directory already exits\n");
            printf("Aborting...\n");
            printf(ANSI_RESET);
            return i;
        }
    }
    return 1;
}
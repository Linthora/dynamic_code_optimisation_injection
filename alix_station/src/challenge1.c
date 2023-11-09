#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// function to optimized
long long fast_exponentiation_long_long(long long x, long long y) {
    // fast exponentiation
    if(y < 0) {
        printf("Error: y must be positive\n");
        return -1;
    }

    long long res = 1;
    while(y > 0) {
        if(y % 2 == 1) {
            res *= x;
        }
        x *= x;
        y /= 2;
    }
    return res;
}

int main(int argc, char *argv[]) {
    
    // pid is the first argument given to the program
    if(argc < 2) {
        printf("Error: pid must be given as argument\n");
        return -1;
    }

    pid_t pid = atoi(argv[1]);
    printf("pid: %i\n", pid);

    // add ptrace attach 0000000000401196

    long result;

    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    int status;
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    //long addr = 0x0000000000401196;

    // open the process memory (read and write)

    // create string with "/proc/" + pid +  "/mem

    char* start = "/proc/";
    char* end = "/mem";
    char* pid_str = argv[1];

    char* path = malloc(strlen(start) + strlen(end) + strlen(pid_str) + 1);
    strcpy(path, start);
    strcat(path, pid_str);
    strcat(path, end);

    printf("path: %s\n", path);

    FILE *fp = fopen(path, "r+");

    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    fclose(fp);

    // enter to continue
    printf("Press enter to continue\n");
    getchar();

    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    ptrace(PTRACE_DETACH, pid, NULL, NULL);

    return EXIT_SUCCESS;
}


// étape 1, faire un trap dans le processus fils
// dans /proc -> ya un "faux" fichier qui contient la mémoire, qu'on a le droit de read or write si on est attaché
// sinon ya TXT  mais moins bien car c'est par mots


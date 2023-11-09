#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

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

    // add ptrace attach 0000000000401196


    ptrace(PTRACE_ATTACH, pid, NULL, NULL);


    printf("I am the parent process\n");

    long addr = 0x0000000000401196;




    // insert a breakpoint
    long data = ptrace(PTRACE_PEEKDATA, pid, addr, NULL); // get the instruction


    long replace_with = (data & 0xFFFFFFFFFFFFFF00) | 0xCC; // what to replace with

    ptrace(PTRACE_POKEDATA, pid, addr, replace_with); // replace the instruction




    // ptrace(PTRACE_DETACH, pid, NULL, NULL);

    return EXIT_SUCCESS;
}


// étape 1, faire un trap dans le processus fils

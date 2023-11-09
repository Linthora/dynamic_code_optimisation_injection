#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

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

    long result;

    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    int status;
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    long addr = 0x0000000000401196;



    // insert a breakpoint
    long data = ptrace(PTRACE_PEEKDATA, pid, addr, NULL); // get the instruction
    assert(data != -1);

    long replace_with = (data & 0xFFFFFFFFFFFFFF00) | 0xCC; // what to replace with
    
    result = ptrace(PTRACE_POKEDATA, pid, addr, replace_with); // replace the instruction
    assert(result == 0);

    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // ptrace(PTRACE_DETACH, pid, NULL, NULL);

    return EXIT_SUCCESS;
}


// étape 1, faire un trap dans le processus fils

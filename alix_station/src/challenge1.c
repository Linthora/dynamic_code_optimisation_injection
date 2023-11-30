#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// function to optimized
long long fast_exponentiation_long_long(long long x, long long y);

void fast_answer();

// get the address of the function to optimize
long get_addr(char* path, char* function_name);

// find the pid of the process to optimize
int find_pid(char* name);

// challenge 1
int challenge1(char * prog_name, char * function_name);


// challenge 2
int challenge2(char * prog_name);

int main(int argc, char *argv[]) {
 
    // prog_name is given in argument

    if(argc < 2) {
        printf("Error: prog_name is missing\n");
        return -1;
    }

    char * prog_name = argv[1];

    char * function_name = "answer";
    challenge1(prog_name, function_name);
    //challenge2(prog_name);
}

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

void fast_answer() {
    printf("Hey ! Gotta go fast_exp(3,100) = %lli\n", fast_exponentiation_long_long(3, 10000));
}

// étape 1, faire un trap dans le processus fils
// dans /proc -> ya un "faux" fichier qui contient la mémoire, qu'on a le droit de read or write si on est attaché
// sinon ya TXT  mais moins bien car c'est par mots

long get_addr(char* path, char* function_name) {
    char* cmd = malloc(1000);
    sprintf(cmd, "objdump -t %s | grep %s | awk '{print $1}' > tmp~", path, function_name); 
    printf("cmd: %s\n", cmd);
    system(cmd);

    free(cmd);

    FILE *fp = fopen("tmp~", "r");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    char line[100];

    fscanf(fp, "%s", line);
    printf("line: %s\n", line);

    long addr = strtol(line, NULL, 16);

    //long addr;
    fclose(fp);

    // delete the tmp file
    system("rm tmp~");


    return addr;
}

int find_pid(char* name) {
    char* cmd = malloc(1000);
    sprintf(cmd, "ps -aux | grep %s | awk '{print $2}' > tmp~", name);
    system(cmd);
    free(cmd);

    int pid;

    FILE *fp = fopen("tmp~", "r");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }
    fscanf(fp, "%i", &pid);

    fclose(fp);
    system("rm tmp~");

    return pid;
}


int challenge1(char * prog_name, char * function_name) {
    int pid = find_pid(prog_name);

    char * prog_where = "../build/prog_to_run";
    long addr = get_addr(prog_where, function_name);

    printf("pid: %i\n", pid);
    printf("addr: %lx\n", addr); 

    long result;

    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    int status;
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    char * path = malloc(100);
    sprintf(path, "/proc/%i/mem", pid);

    printf("path: %s\n", path);

    FILE *fp = fopen(path, "a+");

    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    // put a stop instruction at the address addr

    fseek(fp, addr, SEEK_SET);

    printf("before write\n");

    int instr = 0xCC;

    fwrite( (void *) &instr, 1, sizeof(int), fp);

    printf("after write\n");

    fclose(fp);

    // enter to continue
    printf("Press enter to continue\n");
    getchar();

    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    // assert(result == 0);

    ptrace(PTRACE_DETACH, pid, NULL, NULL);

    free(path);

    return EXIT_SUCCESS;
}



// CHALLENGE 2


int challenge2(char * prog_name) {
    int pid = find_pid(prog_name);

    char * prog_where = "../build/prog_to_run";
    long addr = get_addr(prog_where, "foo");

    printf("pid: %i\n", pid);
    printf("addr: %lx\n", addr); 

    long result;

    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    int status;
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    char * path = malloc(100);
    sprintf(path, "/proc/%i/mem", pid);

    printf("path: %s\n", path);

    FILE *fp = fopen(path, "a+");

    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    // enter to continue
    printf("Press enter to continue\n");
    getchar();

    // Récupération des Registres: Utilisation de ptrace avec PTRACE_GETREGS pour récupérer les valeurs des registres du processus tracé.

    struct user_regs_struct regs;

    // Save the current instruction
    long original_instruction = ptrace(PTRACE_PEEKTEXT, pid, regs.rip, NULL);

    // Calculate the relative address of the function from the next instruction
    long relative_address = addr - (regs.rip + 5);

    // Write the call instruction and the trap instruction
    //ptrace(PTRACE_POKETEXT, pid, regs.rip, (original_instruction & 0xFFFFFFFF00000000L) | 0xccE8000000L | (relative_address & 0xFFFFFFFF));
    // Set the rax register to the address of the function to call
    regs.rax = 0xffd0;

    // Set the rip register to the location of the inserted call instruction
    regs.rip = addr;

    // Set the new register values
    ptrace(PTRACE_SETREGS, pid, NULL, &regs);

    // set a trap after the call instruction
    ptrace(PTRACE_POKETEXT, pid, regs.rip, (original_instruction & 0xFFFFFFFF00000000L) | 0xccE8000000L | (relative_address & 0xFFFFFFFF));

    // Continue the tracee's execution
    ptrace(PTRACE_CONT, pid, NULL, NULL);

    /*// Get the current register values
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    // save the current instruction
    long original_instruction = ptrace(PTRACE_PEEKTEXT, pid, regs.rip, NULL);

    // write the call instruction and the trap instruction
    ptrace(PTRACE_POKETEXT, pid, regs.rip, (original_instruction & 0xFFFFFFFF00000000L) | 0xd0ffcc);

    // Set the rax register to the address of the function to call
    regs.rax = addr;
    
    // Set the new register values
    ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    
    // Continue the tracee's execution
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);*/

    // ...

    // press enter to continue
    printf("Press enter to continue (before detaching)\n");
    getchar();

    // Détacher le processus tracé
    result = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    assert(result == 0);

    // ...

   

    free(path);

    // press enter to continue
    printf("Press enter to continue\n");
    getchar();

    return EXIT_SUCCESS;
}
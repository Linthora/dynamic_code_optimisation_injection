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
int challenge2(char * prog_name, char * function_name);

int main(int argc, char *argv[]) {
 
    // prog_name is given in argument

    if(argc < 2) {
        printf("Error: prog_name is missing\n");
        return -1;
    }

    char * prog_name = argv[1];

    char * function_name = "answer";
    //challenge1(prog_name, function_name);
    challenge2(prog_name, function_name);
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


int challenge2(char * prog_name, char * function_name) {
    int pid = find_pid(prog_name);

    char * prog_where = "../build/prog_to_run";
    long addr_foo = get_addr(prog_where, "foo");
    long addr = get_addr(prog_where, function_name);
    printf("addr_foo: %lx\n", addr_foo);
    printf("addr of %s: %lx\n", function_name, addr);

    printf("pid: %i\n", pid);
    printf("addr: %lx\n", addr); 

    long result;

    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    printf("attached\n");
    //printf("Press enter to continue\n");
    getchar();

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

    // trap at addr
    // insert a call %eax instruction after the trap
    // insert a trap after the call instruction
    printf("before write\n");
    fseek(fp, addr, SEEK_SET);
    
    // byte array
    unsigned char intr[] = { 0xCC, // trap
                    0xFF, 0xD0, // call %eax
                    0xCC // trap
                    };
    fwrite( (void *) intr, 1, sizeof(intr), fp);
    printf("size of intr: %lu\n", sizeof(intr));
    printf("after write\n");
    fflush(fp);
    fclose(fp);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);


    // Get the current register values
    struct user_regs_struct regs;

    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    // save the current eax
    long original_eax = regs.rax;

    // put addr_foo in eax
    regs.rax = addr_foo;

    // Set the new register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    assert(result == 0);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    printf("Press enter to continue (second trap)\n");
    getchar();

    // Get the current register values
    // result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    // restore the eax register
    regs.rax = original_eax;

    // Set the new register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    assert(result == 0);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);
    printf("resumed\n");

    // Détacher le processus tracé
    result = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    assert(result == 0);   

    free(path);

    // press enter to continue
    printf("Press enter to continue\n");
    getchar();

    return EXIT_SUCCESS;
}
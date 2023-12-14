#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// get the address of the function to optimize
long get_addr(char* path, char* function_name);

// find the pid of the process to optimize
int find_pid(char* name);

// challenge 1
int challenge1(char * prog_name, char * function_name);


// challenge 2
int challenge2(char * prog_name, char * function_name);

int challenge3(char *prog_name, char *function_name);

int main(int argc, char *argv[]) {
 
    // prog_name is given in argument

    if(argc < 2) {
        printf("Error: prog_name is missing\n");
        return -1;
    }

    char * prog_name = argv[1];

    char * function_name = "exponentiation_long_long";
    //challenge1(prog_name, function_name);
    //challenge2(prog_name, function_name);
    challenge3(prog_name, function_name);
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

    printf("exit get_addr\n");

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

    printf("before write\n");
    fseek(fp, addr, SEEK_SET);

    // foo is int foo(int * i)

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
    long original_rdi = regs.rdi;

    // put addr_foo in eax
    regs.rax = addr_foo;

    fp = fopen(path, "a+");

    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    int arg = 6;
    regs.rsp -= sizeof(int);

    // Write the argument value to the target process memory
    fseek(fp, regs.rsp, SEEK_SET);
    fwrite((void *)&arg, sizeof(int), 1, fp);
    fflush(fp);
    fclose(fp);

    // Set rdi to the address of the argument value
    regs.rdi = regs.rsp;

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
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    printf("rax: %lli\n", regs.rax);
    printf("Press enter to continue (after print)\n");
    getchar();

    // restore the eax register
    regs.rax = original_eax;
    regs.rdi = original_rdi;
    regs.rsp += sizeof(int);

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

/**
 * Challenge 3
*/

// Challenge 3 - Code Cache
int challenge3(char *prog_name, char *function_name) {
    // code of the function to optimise
    unsigned char code[] = {
          0xf3, 0x0f, 0x1e, 0xfa, 0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0xec, 0x20, 0x48, 0x89, 0x7d, 0xe8, 0x48, 0x89, 0x75, 0xe0
        , 0x48, 0x83, 0x7d, 0xe0, 0x00, 0x79, 0x18, 0x48, 0x8d, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0xc7, 0xe8, 0x00, 0x00, 0x00, 0x00
        , 0x48, 0xc7, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xeb, 0x5c, 0x48, 0xc7, 0x45, 0xf8, 0x01, 0x00, 0x00, 0x00, 0xeb, 0x47
        , 0x48, 0x8b, 0x45, 0xe0, 0x48, 0x99, 0x48, 0xc1, 0xea, 0x3f, 0x48, 0x01, 0xd0, 0x83, 0xe0, 0x01, 0x48, 0x29, 0xd0, 0x48, 0x83, 0xf8, 0x01         
        , 0x75, 0x0d, 0x48, 0x8b, 0x45, 0xf8, 0x48, 0x0f, 0xaf, 0x45, 0xe8, 0x48, 0x89, 0x45, 0xf8, 0x48, 0x8b, 0x45, 0xe8, 0x48, 0x0f, 0xaf, 0xc0          	
        , 0x48, 0x89, 0x45, 0xe8, 0x48, 0x8b, 0x45, 0xe0, 0x48, 0x89, 0xc2, 0x48, 0xc1, 0xea, 0x3f, 0x48, 0x01, 0xd0, 0x48, 0xd1, 0xf8             	
        , 0x48, 0x89, 0x45, 0xe0, 0x48, 0x83, 0x7d, 0xe0, 0x00, 0x7f, 0xb2, 0x48, 0x8b, 0x45, 0xf8, 0xc9, 0xc3
    };

    int pid = find_pid(prog_name);

    char * prog_where = "../build/prog_to_run";
    long addr = get_addr(prog_where, function_name);
    printf("addr: %lx\n", addr);
    printf("pid: %i\n", pid);

    long result;

    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    printf("attached\n");

    printf("Press enter to continue\n");
    getchar();

    int status;
    result = waitpid(pid, &status, 0);
    assert(result == pid);

    char * path = malloc(100);
    sprintf(path, "/proc/%i/mem", pid);

    printf("path: %s\n", path);
    FILE *fp = fopen(path, "rw");

    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    printf("before write\n");
    printf("press enter to continue\n");
    getchar();

    long addr_getpagesize = get_addr(prog_where, "getpagesize");
    long addr_memalign = get_addr(prog_where, "memalign");

    // I need to do something like this:
    // trap
    // call getpagesize -> save the result in a register (the one that will be used as argument for memalign)
    // trap

    // make space on stack for the pointer to pass to memalign
    // call memalign with the result of getpagesize as argument, the address of the pointer on the stack as argument and the size of the code as argument (145)
    // trap
    // get the address of the pointer on the stack

    // trap, call getpagesize, trap; same for memalign
    unsigned char instructions[] = {
        0xCC, // trap
        0xFF, 0xD0, // call %eax
        0xCC, // trap
        0xFF, 0xD0, // call %eax
        0xCC // trap
    };

    unsigned char * save_instructions = malloc(sizeof(instructions));

    printf("size of instructions: %lu\n", sizeof(instructions));
    printf("saving instructions\n");
    // save
    fseek(fp, addr, SEEK_SET);
    fread(save_instructions, 1, sizeof(instructions), fp);
    fflush(fp);
    
    // write
    fseek(fp, addr, SEEK_SET);
    fwrite(instructions, 1, sizeof(instructions), fp); // trap, call getpagesize, trap, call memalign, trap
    fflush(fp);
    fclose(fp);

    // resume until the first trap
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    // Get the current register values
    struct user_regs_struct original_regs;
    struct user_regs_struct regs;
    
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);
    result = ptrace(PTRACE_GETREGS, pid, NULL, &original_regs);
    assert(result == 0);

    // put the address of getpagesize in eax
    regs.rax = addr_getpagesize;

    result = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    assert(result == 0);

    printf("Press enter to resume after writing the instruction\n");
    getchar();

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap (getpagesize)
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    printf("trap 1 caugth\n");

    // TRAP 1 CAUGHT, GETPAGESIZE CALLED, NOW CALL MEMALIGN

    // Get the current register values
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);

    int page_size = regs.rax;
    printf("page_size: %i\n", page_size);

    // need to make space on the stack for the pointer to pass to memalign
    regs.rsp -= sizeof(long);
    // get the address of the pointer on the stack
    regs.rdi = regs.rsp;
    regs.rsi = page_size;
    regs.rdx = sizeof(code);

    // put the address of memalign in eax
    regs.rax = addr_memalign;

    // set the new register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    assert(result == 0);

    printf("Press enter to resume after writing the register for getpagesize\n");
    getchar();

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);


    // wait for the trap (memalign)
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    // TRAP 2 CAUGHT, MEMALIGN CALLED, NOW GET THE ADDRESS OF THE POINTER ON THE STACK

    printf("trap 2 caugth\n");

    // Get the current register values
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);

    // get the address of the pointer on the stack
    long addr_pointer = regs.rdi;

    // see if the address is correct
    printf("addr_pointer: %lx\n", addr_pointer);

    // press enter to continue
    printf("Press enter to continue\n");
    getchar();

    // restore the code
    fp = fopen(path, "rw");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }
    fseek(fp, addr, SEEK_SET);
    fwrite(save_instructions, 1, sizeof(instructions), fp);
    fflush(fp);

    // make a jump to the new code

    fseek(fp, addr_pointer, SEEK_SET);
    // Code du jump absolu : 0x48 0xb8 suivi sur 8 octets de l'adresse à laquelle on veut se brancher
    unsigned char jump[] = {
        0x48, 0xb8
    };

    // write the jump
    fwrite(jump, 1, sizeof(jump), fp);
    fflush(fp);
    // write the addr_pointer
    fwrite(&addr_pointer, 1, sizeof(addr_pointer), fp);
    fflush(fp);
    fclose(fp);

    // change the original registers to directly resume on the jump
    original_regs.rip = addr_pointer;

    // set the new register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &original_regs);
    assert(result == 0);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    return 0;
}

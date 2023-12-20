#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/mman.h>
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

// challenge 3 et 4
int challenge3_4(char *prog_name, char *function_name);

// main
int main(int argc, char *argv[]) {
 
    // prog_name is given in argument

    if(argc < 2) {
        printf("Error: prog_name is missing\n");
        return -1;
    }

    char * prog_name = argv[1];

    // char * function_name = "exponentiation_long_long";
    // return challenge1(prog_name, function_name);

    // char * function_name = "answer";
    // return challenge2(prog_name, function_name);
    
    char * function_name = "exponentiation_long_long";
    return challenge3_4(prog_name, function_name);
}

/**
 * --------------------------------------
 * The definitions of the functions used
 * --------------------------------------
*/

/**
 * Function used to get the address of given function in the given elf file
*/
long get_addr(char* path, char* function_name) {
    // First, we make the command to get the address of the function
    char cmd[1000];

    sprintf(cmd, "nm %s | grep %s | awk '{print $1}'", path, function_name); 
    printf("cmd: %s\n", cmd);

    // Then, we execute the command and extract the address
    FILE *fp = popen(cmd, "r");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    char line[100];
    fscanf(fp, "%s", line);
    pclose(fp);

    long addr = strtol(line, NULL, 16);

    return addr;
}

/**
 * Function used to find the pid of process with given prog_name
*/
int find_pid(char* name) {

    char cmd[1000];
    sprintf(cmd, "ps -aux | grep %s | awk '{print $2}'", name);
    system(cmd);
    
    int pid;

    FILE *fp = popen(cmd, "r");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }
    fscanf(fp, "%i", &pid);
    pclose(fp);

    return pid;
}

/**
 * Challenge 1: put a trap instruction at the beginning of a given function
*/
int challenge1(char * prog_name, char * function_name) {
    // we retrieve the pid
    int pid = find_pid(prog_name);

    // we assume that the program is located here
    char * prog_where = "../build/prog_to_run";

    // we retrieve the address of the given function
    long addr = get_addr(prog_where, function_name);

    printf("pid: %i\n", pid);
    printf("addr: %lx\n", addr); 

    long result;

    // We attach to the process (and make sure it works with assert)
    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    // We wait for the process to stop
    int status;
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    // Now, we need to access the memory of the process
    char path[100];
    sprintf(path, "/proc/%i/mem", pid);

    printf("path: %s\n", path);

    // We access the memory of the process
    FILE *fp = fopen(path, "a+");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    // We go to the address of the function
    fseek(fp, addr, SEEK_SET);

    // The trap instruction is 0xCC
    int trap = 0xCC;

    // Now, we write the trap instruction
    fwrite( (void *) &trap, 1, 1, fp);
    fclose(fp);

    // We resume the process and detach
    // result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    // assert(result == 0);

    // Then we detach
    result = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    assert(result == 0);

    return 0;
}

/**
 * Challenge 2: call a function after trapping our program somewhere. And allow to resume the execution after like nothing happened.
 * 
 * It is to be noted that, unlike in challenge 3/4, we didn't write a trap alone
 * before writing the call trap to avoid problems with the execution of the program.
 * However, it very unlikely to happen and didn't happened any time we tested challenge 2. (And we fixed it in challenge 3/4)
*/
int challenge2(char * prog_name, char * function_name) {
    int pid = find_pid(prog_name);

    char * prog_where = "../build/prog_to_run";
    
    // now, we get the address of the function to call (foo)
    long addr_foo = get_addr(prog_where, "foo");

    long addr = get_addr(prog_where, function_name);
    printf("addr_foo: %lx\n", addr_foo);
    printf("addr of %s: %lx\n", function_name, addr);
    printf("pid: %i\n", pid);

    long result;

    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    int status;
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    printf("attached and stopped\n");

    char path[100];
    sprintf(path, "/proc/%i/mem", pid);

    printf("path: %s\n", path);

    FILE *fp = fopen(path, "rw+");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    // foo is of type (int foo(int * i))

    // byte array containing the instructions to write
    unsigned char instr[] = { 0xCC, // trap
                    0xFF, 0xD0, // call %eax
                    0xCC // trap
                    };

    // byte array containing instruction that will be overwritten    
    unsigned char * save_instr = malloc(sizeof(instr));

    // save the instructions
    fseek(fp, addr, SEEK_SET);
    fread(save_instr, 1, sizeof(instr), fp);
    fflush(fp);
        
    // write the instructions
    fseek(fp, addr, SEEK_SET);
    fwrite( (void *) instr, 1, sizeof(instr), fp);
    fflush(fp);

    fclose(fp);
    
    // resuming after writing the trap call trap
    printf("Press enter to continue (after writing the trap call trap)\n");
    getchar();

    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    printf("resumed and waiting for the 1st trap\n");
    // wait for the trap (first trap) 
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    // First trap caught

    // Get the current register values
    struct user_regs_struct regs;
    struct user_regs_struct original_regs; // used to restore the registers later
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);
    result = ptrace(PTRACE_GETREGS, pid, NULL, &original_regs);
    assert(result == 0);

    // put addr_foo in eax for the call %eax instruction
    regs.rax = addr_foo;

    // Now, we need to write the argument of foo in the stack
    // And then passe the address of the argument to rdi
    
    int arg = 6;

    // make space on the stack for the argument
    regs.rsp -= sizeof(int);
    
    fp = fopen(path, "a+");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    // Write the argument value to the target process stack
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

    // Now, we check that the value of the argument was indeed changed
    // and that the return value is correct
    // Then, we restore the original registers and instructions

    // Get the current register values
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    printf("rax: %lli\n", regs.rax);
    assert(regs.rax == 10); // with arg = 6

    // get the value of the argument
    int value_arg;
    fp = fopen(path, "rw+");
    fseek(fp, regs.rsp, SEEK_SET);
    fread(&value_arg, sizeof(int), 1, fp);
    fflush(fp);

    printf("value_arg: %i\n", value_arg);
    assert(value_arg == 666); // no matter the value of arg

    // Pause, before restoring the registers
    // Usefull to see the actual call of foo
    printf("Press enter to continue (after print)\n");
    getchar();

    // restore the instructions
    fseek(fp, addr, SEEK_SET);
    fwrite(save_instr, 1, sizeof(instr), fp);
    fflush(fp);
    fclose(fp);

    free(save_instr);

    // Restore the register to the original values with the according rip (because of the first trap)
    original_regs.rip = addr;

    // note: if we didn't save the registers, we should also have restored the stack pointer
    // but we don't need to do it since the stack pointer is restored with the registers

    result = ptrace(PTRACE_SETREGS, pid, NULL, &original_regs);
    assert(result == 0);

    // resume and detach
    result = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    assert(result == 0);
    printf("resumed\n");

    return 0;
}

/**
 * Challenge 3 and 4: replace, or more precisely redirect, a function with another one.
 * 
 * Here, it will be the function exponentiation_long_long with fast_exponentiation_long_long.
 * 
 * Note that we ensure at each step that the called system calls are successful with assert.
*/
int challenge3_4(char *prog_name, char *function_name) {
    
    // code of the function to optimise
    // consists of the byte array of the instructions of the function in function_optimized.c
    unsigned char code[] = {
          0xf3, 0x0f, 0x1e, 0xfa, 0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0xec, 0x20, 0x48, 0x89, 0x7d, 0xe8, 0x48, 0x89, 0x75, 0xe0
        , 0x48, 0x83, 0x7d, 0xe0, 0x00, 0x79, 0x18, 0x48, 0x8d, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0xc7, 0xe8, 0x00, 0x00, 0x00, 0x00
        , 0x48, 0xc7, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xeb, 0x5c, 0x48, 0xc7, 0x45, 0xf8, 0x01, 0x00, 0x00, 0x00, 0xeb, 0x47
        , 0x48, 0x8b, 0x45, 0xe0, 0x48, 0x99, 0x48, 0xc1, 0xea, 0x3f, 0x48, 0x01, 0xd0, 0x83, 0xe0, 0x01, 0x48, 0x29, 0xd0, 0x48, 0x83, 0xf8, 0x01         
        , 0x75, 0x0d, 0x48, 0x8b, 0x45, 0xf8, 0x48, 0x0f, 0xaf, 0x45, 0xe8, 0x48, 0x89, 0x45, 0xf8, 0x48, 0x8b, 0x45, 0xe8, 0x48, 0x0f, 0xaf, 0xc0          	
        , 0x48, 0x89, 0x45, 0xe8, 0x48, 0x8b, 0x45, 0xe0, 0x48, 0x89, 0xc2, 0x48, 0xc1, 0xea, 0x3f, 0x48, 0x01, 0xd0, 0x48, 0xd1, 0xf8             	
        , 0x48, 0x89, 0x45, 0xe0, 0x48, 0x83, 0x7d, 0xe0, 0x00, 0x7f, 0xb2, 0x48, 0x8b, 0x45, 0xf8, 0xc9, 0xc3
    };

    printf("prog_name: %s\n", prog_name);
    printf("function_name: %s\n", function_name);
    printf("size of code: %lu\n", sizeof(code));

    // find the pid of the process to optimize
    int pid = find_pid(prog_name);
    printf("pid: %i\n", pid);

    // get the address of the function to optimize
    char * prog_where = "../build/prog_to_run";
    long addr = get_addr(prog_where, function_name);

    printf("addr of %s: %lx\n", function_name, addr);
    printf("pid: %i\n", pid);

    long result;

    // attach to the process
    result = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    assert(result == 0);

    int status;
    result = waitpid(pid, &status, 0);
    assert(result == pid);
    printf("status: %i\n", status);

    printf("attached and stopped\n");

    char * path = malloc(100);
    sprintf(path, "/proc/%i/mem", pid);

    /**
     *  We now write only 1 trap instruction at the address addr.
     *  The reason why we don't write the whole trap call (getpagesize) trap call (memalign) trap is simple:
     *  If we were to write directly multiple instructions, as we are executing the program that calls the function
     *  we want to overwrite constantly, we would risk resuming the execution in the middle of the newly written instructions
     *  and cause problems. (for example, stopping on the second trap and not the first one) 
     */
    printf("path: %s\n", path);
    FILE *fp = fopen(path, "rw+");

    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    unsigned char trap = 0xCC;
    unsigned char * save_1st_trap = malloc(sizeof(trap)); // This is useless, because this part is going to be overwritten by the trampoline anyway, but to train and good practice

    // save 
    fseek(fp, addr, SEEK_SET);
    fread(save_1st_trap, 1, sizeof(trap), fp);
    fflush(fp);
    // write
    fseek(fp, addr, SEEK_SET);
    fwrite(&trap, 1, 1, fp);
    fflush(fp);
    fclose(fp);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    // Trap 1 caught

    // We now need to write call getpagesize (trap call) trap call (memalign) trap then call (mprotect) trap then make the necessary
    // change to the registers to bring back the program to the beginning of the function to optimize then rewrite the saved instructions
    // and finally write a jump to the new code. 

    // get the addresses of the functions we need. Works because the program is compiled statically
    long addr_getpagesize = get_addr(prog_where, "getpagesize");
    long addr_memalign = get_addr(prog_where, "posix_memalign");
    long addr_mprotect = get_addr(prog_where, "mprotect");

    printf("addr_getpagesize: %lx\n", addr_getpagesize);
    printf("addr_memalign: %lx\n", addr_memalign);
    printf("addr_mprotect: %lx\n", addr_mprotect);

    // the instructions to write
    unsigned char instructions[] = {
        0xFF, 0xD0, // call %eax; getpagesize
        0xCC, // trap
        0xFF, 0xD0, // call %eax; memalign
        0xCC, // trap
        0xFF, 0xD0, // call %eax; mprotect
        0xCC // trap
    };

    unsigned char * save_instructions = malloc(sizeof(instructions));

    fp = fopen(path, "rw+");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    printf("size of instructions: %lu\n", sizeof(instructions));
    printf("saving instructions\n");
    
    // save the instructions, starting at addr + 1 (because of the first trap)
    fseek(fp, addr + 1, SEEK_SET);
    fread(save_instructions, 1, sizeof(instructions), fp);
    fflush(fp);
    
    // write the instructions
    fseek(fp, addr + 1, SEEK_SET);
    fwrite(instructions, 1, sizeof(instructions), fp);
    fflush(fp);
    fclose(fp);

    // before resuming, we need to change and save the registers to make the call to getpagesize

    // same as in challenge 2, we also store the original registers to restore them later
    struct user_regs_struct original_regs;
    struct user_regs_struct regs;

    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);
    result = ptrace(PTRACE_GETREGS, pid, NULL, &original_regs);
    assert(result == 0);

    // put the address of getpagesize in eax
    regs.rax = addr_getpagesize;
    // set the new register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    assert(result == 0);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap (getpagesize)
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    // Trap 2 caught
    // We get the result of getpagesize
    // Then, we need to call memalign with the correct arguments

    // Get the current register values
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);

    size_t page_size = regs.rax; // on my computer, it is 4096
    printf("page_size: %li\n", page_size); 

    size_t code_size = sizeof(code);
    printf("code_size: %li\n", code_size);

    // we need to make space on the stack for the pointer to pass to memalign
    regs.rsp -= sizeof(void *);

    // get the address of the pointer on the stack
    regs.rdi = regs.rsp;
    regs.rsi = page_size;
    regs.rdx = code_size;
    
    // put the address of memalign in eax
    regs.rax = addr_memalign;

    // set the new register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    assert(result == 0);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap (memalign)
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);

    // Trap 3 caught
    // We get the result of memalign and assert that the call to memalign returned 0
    // Then, we need to call mprotect with the correct arguments

    // Get the current register values
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);

    fopen(path, "rw+");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    // assert that the call to memalign returned 0
    assert(regs.rax == 0);

    // get the address of the pointer on the stack
    long long unsigned int addr_pointer;

    fseek(fp, regs.rsp, SEEK_SET);
    fread(&addr_pointer, 1, sizeof(addr_pointer), fp);
    fflush(fp);

    printf("addr_pointer: %lld\n", addr_pointer);

    fclose(fp);

    // give the correct arguments to mprotect
    regs.rdi = addr_pointer;
    regs.rsi = sizeof(code);
    regs.rdx = PROT_WRITE | PROT_EXEC;
    regs.rax = addr_mprotect;

    // set the new register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    assert(result == 0);

    // resume
    result = ptrace(PTRACE_CONT, pid, NULL, NULL);
    assert(result == 0);

    // wait for the trap (mprotect)
    result = waitpid(pid, &status, 0);
    printf("status: %i\n", status);
    assert(result == pid);


    // Trap 4 caught
    // We get the result of mprotect and assert that the call to mprotect returned 0
    // Then, we need to both:
    // - write the new code in the space allocated by memalign
    // - restore the original instructions in case we overlapped on another function (not the case here though)
    // - write a jump from the original function to the new code
    
    // Get the current register values
    result = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    assert(result == 0);

    // assert that the call to mprotect returned 0
    assert(regs.rax == 0);

    fopen(path, "rw+");
    if(fp == NULL) {
        printf("Error: cannot open file\n");
        return -1;
    }

    // write the code at the address of the pointer
    fseek(fp, addr_pointer, SEEK_SET);
    fwrite(code, 1, sizeof(code), fp);
    fflush(fp);

    // restore the instructions
    fseek(fp, addr, SEEK_SET);
    fwrite(save_1st_trap, 1, sizeof(trap), fp);
    fwrite(save_instructions, 1, sizeof(instructions), fp);
    fflush(fp);

    // write the jump: 0x48 0xb8 addr_pointer 0xff 0xe0
    unsigned char jump[] = {
        0x48, 0xb8
    };

    fseek(fp, addr, SEEK_SET);
    fwrite(jump, 1, 2, fp);

    fwrite(&addr_pointer, 1, 8, fp);

    unsigned char end_jump[] = {
        0xff, 0xe0
    };

    // write the end of the jump
    fwrite(end_jump, 1, 2, fp);
    fflush(fp);

    fclose(fp);

    // set the original registers with the correct rip pointing at the beginning of the function to optimize
    // (we have to do it because of the first trap)
    original_regs.rip = addr; 

    // set the new (original updated) register values
    result = ptrace(PTRACE_SETREGS, pid, NULL, &original_regs);
    assert(result == 0);

    // resume and detach
    result = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    assert(result == 0);

    // free the memory
    free(path);
    free(save_1st_trap);
    free(save_instructions);

    return 0;
}

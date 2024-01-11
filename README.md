# Project Summary: Dynamic Code Optimization and Injection

The "Dynamic Code Optimization and Injection" project focuses on enhancing the performance of a target program through dynamic code modification and injection techniques. The project involves two main components: the traced process (prog_to_run) and the tracing process (challenges). The traced process contains a function, exponentiation_long_long, targeted for optimization. The tracing process orchestrates various challenges, each demonstrating a unique aspect of dynamic code manipulation. It employs techniques such as interrupting and modifying program execution, allocating memory for optimized code, and utilizing trampolines to seamlessly transition between original and optimized code. The culmination of these challenges results in a significantly accelerated runtime for the traced program. The project showcases advanced system-level programming skills, including low-level code analysis, memory manipulation, and dynamic optimization strategies.

# Codename-Plage: Dynamic Code Optimization and Injection

In this project, we have completed challenges 1, 2, 3, and 4.

The source code for the traced process is located in the file `src/prog.c`:
- The function to optimize is `exponentiation_long_long`.
- The `answer` function calls it a certain number of times, and it is launched multiple times, each time being timed.
- The function called in challenge 2 is the `foo` function.
- The program is compiled statically.

The source code for the tracing process is in the file `src/challenges.c`.
- It is divided into three functions: `challenge1` for challenge 1, `challenge2`, and `challenge3_4` for challenges 3 and 4.
- It contains some utility functions to retrieve the address of a given function, as well as the PID of the given program.

The optimized function is in the file `src/function_optimized.c`.
- The assembler code can be extracted using `objdump -d build/function_optimized.o | awk '/<fast_exponentiation_long_long>:/,/^$/' | grep -v '^$'`.

## Compiling the Project

To compile the project, simply run the `compil.sh` script in the root directory (`./compil.sh`).

This will compile three programs: two executables, one for tracing and one for the tracer, and one object file for the optimized function.

Note: The executable for `prog.c` is named `prog_to_run`.

## Running the Project

To run the project, first navigate to the root of the project.

Then, in one terminal, run the first program: `./build/prog_to_run`.

In another terminal, run the tracing program with: `./build/challenges prog_to_run`.
By default, the tracing process launches challenges 3 and 4, but you can add an extra argument to choose challenge 1 or 2 (or 3 for challenges 3 and 4):
- `./build/challenges prog_to_run 1`: challenge 1
- `./build/challenges prog_to_run 2`: challenge 2
- `./build/challenges prog_to_run 3`: challenges 3 and 4

Once the tracing program is launched, it performs the operations required in the given challenge:
1) Challenge 1: stops the trace with a trap.
2) Challenge 2: catches the program with a trap, makes it call a function with an argument that is a pointer, catches it afterward, and verifies the results (including the modification of the pointer by the function). Then, it restores the correct execution of the program before pausing.
3) Challenges 3 and 4:
    - catch the program
    - allocate space on the HEAP to write the optimized version of the function with `posix_memalign`
    - grant execution rights to this allocated memory space with `mprotect`
    - write the code of the optimized function at this location
    - restore any overwritten instructions (in our case, given the size of the function, this is not necessary, but we did it nonetheless to practice good habits)
    - write a trampoline (jump) to replace the execution of the non-optimized function with the execution of the optimized function just written in memory
    - resume the process where it was paused, namely the call to this function

You will notice that once the program is launched and finished, the time taken by the traced program through the `answer` call will be accelerated (by a factor of 10 on our machines).

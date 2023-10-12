#include <stdio.h>
#include <sys/ptrace.h>

void main() {
    ptrace(PTRACE_ATTACH);
}
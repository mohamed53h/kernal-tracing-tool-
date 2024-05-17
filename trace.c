#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/types.h>

void print_syscall(pid_t child) {
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);

    long syscall_num = regs.orig_rax;
    long arg1 = regs.rdi;
    long arg2 = regs.rsi;
    long arg3 = regs.rdx;
    long arg4 = regs.r10;
    long arg5 = regs.r8;
    long arg6 = regs.r9;

    printf("Syscall %ld(", syscall_num);
    printf("%ld, %ld, %ld, %ld, %ld, %ld)\n", arg1, arg2, arg3, arg4, arg5, arg6);
}

int main(int argc, char *argv[]) {
    pid_t child;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    child = fork();
    if (child == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child == 0) {
        // Child process: execute the target program
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            perror("ptrace");
            exit(EXIT_FAILURE);
        }
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: trace the child
        int status;
        while (1) {
            // Wait for the child to stop
            wait(&status);

            if (WIFEXITED(status)) {
                printf("Child exited\n");
                break;
            }

            // Print syscall information
            print_syscall(child);

            // Continue the child process
            ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        }
    }
    
    return 0;
}


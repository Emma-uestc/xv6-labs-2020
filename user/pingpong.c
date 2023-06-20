// pingpong.c
#include "kernel/types.h"
#include "user/user.h"

int main() {
    int pipes1[2], pipes2[2];
    int pid;
    char buf[1];
    // parent write to pipes1[1], child read from pipes1[0]
    // child write to pipes2[1], parent read from pipes2[0]
    pipe(pipes1);
    pipe(pipes2);
    pid = fork();
    if(pid == -1)
        exit(1);
    if(pid == 0) {
        // child process
        int childPid = getpid();
        close(pipes1[1]);
        close(pipes2[0]);
        int n = read(pipes1[0], buf, sizeof(buf));
        if (n == -1)
            exit(1);
        printf("%d: received ping\n", childPid);
        if (write(pipes2[1], buf, n) == -1)
            exit(1);
        close(pipes1[0]);
        close(pipes2[1]);
        exit(0);
    } else {
        // parent process
        close(pipes1[0]);
        close(pipes2[1]);
        if(write(pipes1[1], "a", 1) == -1)
            exit(1);
        int n = read(pipes2[0], buf, sizeof(buf));
        if (n == -1)
            exit(1);
        int parentPid = getpid();
        printf("%d: received pong\n", parentPid);
        close(pipes1[1]);
        close(pipes2[0]);
        exit(1);
    }
}

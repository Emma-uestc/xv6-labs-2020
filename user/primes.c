#include "kernel/types.h"
#include "user/user.h"

#define MAX_PRIME 35

void sieve(int read_fd) {
    int prime;
    int n;
    int write_fd;

    if (read(read_fd, &prime, sizeof(int)) == 0) {
        // If no more input, close the pipe fd and exit
        close(read_fd);
        exit(0);
    }

    printf("prime %d\n", prime); // print the prime in the end

    // Create a new pipe for the next stage
    int pipes[2];
    pipe(pipes);
    write_fd = pipes[1];

    int pid = fork(); // Fork a new process to continue the sieve process
    if (pid == 0) {
        // Child process
        close(pipes[1]); // Child process only need read the data from pipes, so close the write end
        sieve(pipes[0]); // Child process as the parent process for the next stage process,so call the pipes read end
    } else {
        // Parent process
        close(pipes[0]); // parent process do not need read data from pipes fd, so close the read end in the parent process
        // Filter out multiples of the current prime
        while (read(read_fd, &n, sizeof(int)) != 0) {
            if (n % prime) {
                write(write_fd, &n, sizeof(int)); // could devide by the prime, n is the next prime, write to the pipe
            }
        }
        close(read_fd);
        close(write_fd);
        wait(0); // wait the child process complete
        exit(0);
    }
}

int main() {
    int pipes[2];
    pipe(pipes);
    if (fork() == 0) {
        // Child process
        close(pipes[1]); // Close the write fd in the child 
        sieve(pipes[0]);
    } else {
        for (int i = 2; i <= MAX_PRIME; i++) {
            write(pipes[1], &i, sizeof(int));
        }
        close(pipes[1]);
        wait((int *) 0);
        exit(0);
    }

    return 0;
}

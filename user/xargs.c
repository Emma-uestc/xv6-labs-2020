#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"


#define MAX_LEN 1024

int main (int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }

    char *cmd = argv[1];
    char *params[MAXARG];
    char line[MAX_LEN];
    int n, pos = 0;
    // Copy command line params after the command
    for (int i = 1; i < argc; i++) {
        params[pos++] = argv[i];
    }

    // Read command line from standard input and run the command for each line
    while (( n = read(0, line, MAX_LEN)) > 0) {
        int pid = fork();
        if (pid == 0) {
            char *buf = (char*) malloc(sizeof(line));
            int idx = 0;
            for (int i = 0; i < n; i++) {
                if (line[i] == ' ' || line[i] == '\n') {
                    buf[idx] = 0;
                    params[pos++] = buf;
                    idx = 0;
                    buf = (char*) malloc(sizeof(line));
                } else {
                    buf[idx++] = line[i];
                }
            }
            buf[idx] = 0;
            params[pos] = 0;
            exec(cmd, params);
        } else if (pid > 0) {
            wait((int *) 0);
        } else {
            fprintf(2, "xargs: fork failed\n");
            exit(1);
        }
    }
    exit(0);
}

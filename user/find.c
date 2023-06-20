#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAX_PATH_LEN 512

char *fmtname(char *path) {

    static char buf[DIRSIZ + 1];
    char *p;
    for(p = path + strlen(path); p >= path && *p != '/'; p--); 
    p ++;
    if (strlen(p) >= DIRSIZ)
	    return p;
    memmove(buf, p, strlen(p));
    buf[strlen(p)] = 0;
    return buf;
}

void find(char *path, char *name) {

    int fd;
    char buf[MAX_PATH_LEN],*p;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find:  open %s error\n", path);
        exit(1);
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find:  stat %s error\n", path);
        close(fd);
        exit(1);
    }

    switch(st.type) {
        case T_FILE: // if file and match the target name, print the filename
            if(strcmp(fmtname(path), name) == 0)
                printf("%s\n", path);
            break;
        case T_DIR: // if dir, recursive call find()
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
                fprintf(2, "find: path too long\n");
                break;
            } 

            // add '/'
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0) // is dir entry
                    continue;
                // add de.name to path
                memmove(p, &de.name, DIRSIZ);
                p[DIRSIZ] = 0;

                // do not find . and ..
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;
                find(buf, name);
            }
            break;
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: find <path> <name>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
    return 0;
}

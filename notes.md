# Lab  
## lab 1  
### qemu 环境安装  
按照官方手册安装工具链
`sudo apt-get install git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu `
测试安装
`qemu-system-riscv64 --version`
```
QEMU emulator version 4.2.1 (Debian 1:4.2-3ubuntu6.26)
Copyright (c) 2003-2019 Fabrice Bellard and the QEMU Project developers
```
克隆xv6实验仓库
```
git clone git://g.csail.mit.edu/xv6-labs-2020
cd xv6-labs-2020
git checkout util
```
编译
`make qemu`
**官方提示说qemu-system-misc 在编译时会夯住，我这里，没有，如果有的话，可以按照官方手册提示移除新版qemu-system-misc，安装旧版进行编译**
编译成功并进入xv6操作系统的shell,可以看到最后信息如下  
```
riscv64-linux-gnu-objdump -t user/_wc | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > user/wc.sym
riscv64-linux-gnu-gcc -Wall -Werror -O -fno-omit-frame-pointer -ggdb -DSOL_UTIL -MD -mcmodel=medany -ffreestanding -fno-common -nostdlib -mno-relax -I. -fno-stack-protector -fno-pie -no-pie   -c -o user/zombie.o user/zombie.c
riscv64-linux-gnu-ld -z max-page-size=4096 -N -e main -Ttext 0 -o user/_zombie user/zombie.o user/ulib.o user/usys.o user/printf.o user/umalloc.o
riscv64-linux-gnu-objdump -S user/_zombie > user/zombie.asm
riscv64-linux-gnu-objdump -t user/_zombie | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > user/zombie.sym
mkfs/mkfs fs.img README  user/xargstest.sh user/_cat user/_echo user/_forktest user/_grep user/_init user/_kill user/_ln user/_ls user/_mkdir user/_rm user/_sh user/_stressfs user/_usertests user/_grind user/_wc user/_zombie 
nmeta 46 (boot, super, log blocks 30 inode blocks 13, bitmap blocks 1) blocks 954 total 1000
balloc: first 590 blocks have been allocated
balloc: write bitmap block at sector 45
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
```
按`Ctrl-a x.`退出qemu 环境，再执行`make qemu` 进入，之后就只会输出以下信息  
```
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
```
## Lab 1  
* `sleep.c`  
按照官网提示，`sleep`函数调用系统内核`sleep` 函数，并要求用户输入sleep 的时间，如果没有输入时间则返回错误，这里我们调用系统函数`exit(1)异常退出，shell终端输入的是字符串，我们使用`atoi`函数将字符串转换为整数，`sleep.c`如下  
```
#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(2, "usage: sleep [ticks num] \n");
        exit(1);
    }
    int ticks = atoi(argv[1]);
    int ret = sleep(ticks);
    exit(ret);
}
```  
将`sleep.c`放在`user`下，在Makefile `UPROGS`中包含`sleep`，执行`make qemu`重新编译,这样我们将sleep编译进了xv6系统，可以在xv6中执行`sleep`了
```
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2059
xargstest.sh   2 3 93
cat            2 4 23904
echo           2 5 22728
forktest       2 6 13088
grep           2 7 27256
init           2 8 23832
kill           2 9 22704
ln             2 10 22656
ls             2 11 26136
mkdir          2 12 22800
rm             2 13 22792
sh             2 14 41664
stressfs       2 15 23808
usertests      2 16 147440
grind          2 17 37920
wc             2 18 25040
zombie         2 19 22192
sleep          2 20 22728
console        3 21 0
$ sleep 10
```
跟官方文档给出的一样，`sleep 10`是没有任何返回的，如果不带参数是会报错的，退出qemu后，执行`./grade-lab-util sleep` 进行lab打分，直接`./grade-lab-util`会对所有lab进行打分。  
![image]  
* `pingpong.c`  
```
#include "kernel/types.h"
#include "user/user.h"

int main() {
    int pipes1[2], pipes2[2];
    int pid;
    char buf[1];
    // parent write to pipes1[1], child read from pipes1[0]
    // child write to pipes2[1], parent read from pipes2[0]
    pipe(pipes1); // parent -- > child
    pipe(pipes2); // child -- > parent
    pid = fork();
    if(pid == -1)
        exit(1);
    if(pid == 0) {
        // child process
        int childPid = getpid();
        close(pipes1[1]);
        close(pipes2[0]);
        int n = read(pipes1[0], buf, sizeof(buf)); // 子进程读取来自管道的数据，并在读错误时退出
        if (n == -1)
            exit(1);
        printf("%d: received ping\n", childPid); // 
        if (write(pipes2[1], buf, n) == -1)
            exit(1);
        close(pipes1[0]);
        close(pipes2[1]);
        exit(0);
    } else {
        // parent process
        close(pipes1[0]);
        close(pipes2[1]);
        if(write(pipes1[1], "a", 1) == -1) // 父进程将数据写入管道,并判断如果出现写错误退出
            exit(1);
        int n = read(pipes2[0], buf, sizeof(buf)); // 读取子进程发来的数据
        if (n == -1)
            exit(1);
        int parentPid = getpid(); // 子进程收到数据后输出 pong
        printf("%d: received pong\n", parentPid);
        close(pipes1[1]);
        close(pipes2[0]);
        exit(1);
    }
}
```  
  * pipe的用法  
  创建`pipe` 会得到一个长度为 2 的 int 数组 `p[2]`，其中`p[0]` 用于从管道读取fd，`p[1]` 用于向管道写入数据的fd
* primes  
这道题使用了很多进程和管道，通过多stage方式筛掉某个质数所有倍数的方法找到质数，[这篇文章](http://swtch.com/~rsc/thread/) 有详细讲述原理  
`主线程：生成 `$n \in [2,35]$` -> 子进程1：筛掉所有2的倍数 -> 子进程2：筛掉所有3的倍数 -> 子进程3：筛掉所有5的倍数 -> ...`
每一个stage 以当前集合中最小的数字作为质数输出（每个stage中集合中最小的数一定是一个质数，因为它没有任何比它小的数可以被筛掉），并筛掉输入中该质数的所有倍数，然后将剩下的数传递给下一个stage，最后形成一条子进程链，由于每个子进程都调用了  
`wait(0)`等待其子进程完成，所以在最后一个stage完成的时候，沿着链backword依次退出各个进程。  
```
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
```
这题需要注意的就是关闭用不到的fd，否则在运行时会爆掉，因为`fork`会将父进程所有的fd都复制到子进程汇总，   
xv6每个进程可以打印的fd总数只有16个（`kernel/param.h`中的`#define NOFILE       16  // open files per process`和 `kernel/proc.h`中的`struct file *ofile[NOFILE];` 有指定）
* find  
这个跟`ls`基本原理是一样的，可以直接从`ls`中改造得到  
```
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
```
## Lab 2  
* `syscall.c` 代码阅读  
阅读 xv6 4.3、4.4 以及结合代码可知，内核实现系统调用必须要找到用户代码传递的参数，就是通过`syscall.c`中的函数实现找到相关地址，并将地址中的 参数复制到内核内存中  
`fetchaddr`函数用于从当前进程中查找地址并将查找到的地址处的值复制到ip 指针指向的位置，如果复制失败则返回 -1  
`fetchstr`函数用于从当前进程中addr处获取一个非空字符串并使用`copyinstr`函数复制到buf中，并返回字符串的长度； 
`argint`、`argaddr`、`argfd`函数从trap 帧中查找第n个系统调用参数分别作为整数、指针、字符串存储在ip 指针指定的位置，ip 通过`argraw`函数获取到。
`argraw` 函数获取当前进程中系统调用的第n个参数的原始值，根据参数索引从trapframe中找到对应的寄存器的值。  
  比如用户代码将`exec`的参数放在寄存器`a0`和`a1`中，并将系统调用号放在`a7`中，系统调用号与`syscalls`数组中的条目是相匹配的，系统调用实现函数返回时，`syscall`将其返回值记录在`p->trapframe->a0`中。  
* `proc.c` 代码阅读  
首先调用`procinit` 函数初始化进程表，包括锁、状态、内核栈的初始化  
`cpuid()` 显然是获取当前进程的id  
`mycpu()`:返回当前CPU，cpu 的 struct 在`proc.h`中定义如下，**此时中断必须是禁止的**    
  ```
  // Per-CPU state.
  struct cpu {
    struct proc *proc;          // The process running on this cpu, or null.
    struct context context;     // swtch() here to enter scheduler().
    int noff;                   // Depth of push_off() nesting.
    int intena;                 // Were interrupts enabled before push_off()?
  };
  ```
* myproc(void)  
返回当前进程的proc  
* `int allocpid()`: 为新的进程分配一个进程号  
* `allocproc(void)`: 在进程表中查找一个未使用的进程，如果查找到对其进行初始化  
* `freeproc(struct proc *p)`:释放进程及其所占用的资源  
* `pagetable_t proc_pagetable(struct proc *p)`:为指定进程创建用户页表，不包括用户内存但包含trampoline pages  
* `proc_freepagetable(pagetable_t pagetable, uint64 sz)`:释放进程页表及其所引用的物理内存  
* `void userinit(void)`: 设置第一个用户进程，并为该进程分配一个进程页表，分配一个用户页，并将初始化指令和数据拷贝到用户页中  
* `int growproc(int n)`: 增加或减少用户内存大小，以byte为单位  
* `int fork(void)`: 以当前进程为父进程创建一个新的进程，拷贝父进程的内存和上下文  
* `void reparent(struct proc *p)`: 将僵尸进程的父进程的权利交给init 进程，用于回收僵尸进程  
* `void exit(int status)`: 退出当前进程并将其设置为僵尸状态  
  exit主要的工作是，记录调用进程的退出状态，释放一定的资源，把当前进程的所有子进程托管给init进程，然后唤醒当前进程的父进程，将当前进程状态设为ZOMBIE，最后让出CPU。值得注意的是一些上锁的部分，调用exit的进程在设置状态并且唤醒父进程时，  
  必须持有父进程的锁，这是为了防止父进程出现唤醒丢失。调用exit的进程自己也要持有自己的锁，因为进程有一段时间状态是ZOMBIE，但我们实际上还在运行它，因此不应该让父进程发现并释放这个子进程。这里遵守同样的上锁规则，先父进程后子进程，  
  防止死锁发生。[来源](https://juejin.cn/post/7235074521777225765)  
* `int wait(uint64 addr)`:等待子进程exit并返回子进程的PID。  
  在子进程调用exit终止时，父进程可能已经在wait调用上被挂起，或者正在处理其它的工作，如果是后者，那么下一次wait调用应该要能发现子进程的终止，即使子进程已经调用exit很久了。xv6为了让父进程的wait发现子进程已经终止，  
  在子进程exit的时候，将其运行状态设置为ZOMBIE，然后wait就会注意到这个终止的子进程，并将该子进程标记为UNUSED，复制子进程的退出状态，并且返回子进程PID给父进程。如果父进程比子进程先exit，那么它的子进程都会托管给init进程  
（第一个用户进程，第二个是shell），即init进程现在是它们的父进程，init进程（user/init.c）就是在循环中不断地调用wait，如下所示，以释放这些被托管给它的终止子进程的资源。**因此，每个子进程终止并退出后，都由它的父进程清理释放它们。  
**在实现这两个接口的时候，要注意wait和exit，又或是exit和exit之间可能会出现竞争条件或死锁**。  
  wait使用调用进程的p->lock作为条件锁，以防止唤醒丢失。wait在开始时先获取调用进程的p->lock，然后在一个循环中扫描所有进程，如果发现是它的子进程，就获取子进程的锁np->lock，并检查子进程状态，  
  如果状态是ZOMBIE，那么就将子进程的退出状态复制到wait传入的地址addr，并调用freeproc清理子进程的资源和进程结构，最后释放np->lock和p->lock，并且返回退出子进程的pid。如果wait发现自己没有子  
  进程，就会直接返回；如果它的子进程都没有终止，那么wait接下来就会调用sleep挂起自己，释放调用进程的锁p->lock，等待它的其中一个子进程调用exit终止。注意到wait在一段时间内同时持有两把锁，而  
  xv6规定的顺序是，先对父进程上锁，再对子进程上锁，以防止死锁发生。[来源](https://juejin.cn/post/7235074521777225765)  
* `void scheduler(void)`: CPU进程调度器，每个CPU在启动后都会调用调度器。  



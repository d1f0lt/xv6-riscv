#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"

char buffer[PGSIZE];
int curBufferPos = 0;

void flush(int desc) {
    if (curBufferPos == 0) return;
    // printf("Скидываю в буффер: '%s'\n", buffer);
    if (write(desc, buffer, curBufferPos) != curBufferPos) {
        fprintf(2, "Error when writing to pipe");
        close(desc);
        exit(1);
    } 
    curBufferPos = 0;
}

void writeWithBuffer(int desc, const void *s, int cnt) {
    if (cnt + curBufferPos >= PGSIZE)
        flush(desc);
    
    memmove(&buffer[curBufferPos], (const char *)s, cnt);
    curBufferPos += cnt;
}

int main(int argc, char *argv[]) {
    // printf("argc = %d\n", argc);
    int fd[2];

    int pid = fork();
    if (pid < 0) return 1;

    if (pid == 0) {
        close(fd[1]);

        close(0);
        if (dup(fd[0]) != 0) {
            fprintf(2, "dup failed\n");
            exit(1);
        }

        close(fd[0]);

        char *argv[] = {"/wc", 0};
        exec("/wc", argv);
        fprintf(2, "Error when trying to use wc");
        return 1;
    } else {
        close(fd[0]);

        for (int i = 1; i < argc; ++i) {
            char *arg = argv[i];
            writeWithBuffer(fd[1], arg, strlen(arg));
            writeWithBuffer(fd[1], "\n", strlen("\n"));
        }
        flush(fd[1]);

        close(fd[1]);

        int returnCode = 0;
        wait(&returnCode);
        return returnCode;
    }
}
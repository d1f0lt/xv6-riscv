#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"

char buffer[PGSIZE];
int curBufferPos = 0;

int write_all(int desc, const char *s, int cnt) {
    int total = 0;
    while (total < cnt) {
        int written = write(desc, s + total, cnt - total);
        if (written <= 0)
            return 1;
        total += written;
    }
    return 0;
}

int flush(int desc) {
    if (curBufferPos == 0)
        return 0;
    // printf("Скидываю в буффер: '%s'\n", buffer);
    if (write_all(desc, buffer, curBufferPos) != 0)
        return 1;
        
    curBufferPos = 0;
    return 0;
}

int writeWithBuffer(int desc, const void *s, int cnt) {
    if (cnt + curBufferPos >= PGSIZE)
        if (flush(desc) != 0)
            return 1;
    
    memmove(&buffer[curBufferPos], (const char *)s, cnt);
    curBufferPos += cnt;
    return 0;
}

void check_close(int code) {
    if (code < 0) {
        fprintf(2, "close failed\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    // printf("argc = %d\n", argc);
    int fd[2];

    if (pipe(fd) < 0) {
        fprintf(2, "pipe failed\n");
        return 1;
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        if (close(fd[0]) < 0)
            fprintf(2, "close failed\n");
        check_close(close(fd[1]));
        return 1;
    }

    if (pid == 0) {
        check_close(close(fd[1]));
        check_close(close(0));

        if (dup(fd[0]) != 0) {
            fprintf(2, "dup failed\n");
            check_close(close(fd[0]));
            exit(1);
        }

        check_close(close(fd[0]));

        char *argv[] = {"/wc", 0};
        exec("/wc", argv);
        fprintf(2, "Error when trying to use wc");
        return 1;
    } else {
        if (close(fd[0]) < 0) {
            fprintf(2, "close failed\n");
            check_close(close(fd[1]));
            return 1;
        }

        for (int i = 1; i < argc; ++i) {
            char *arg = argv[i];
            if (writeWithBuffer(fd[1], arg, strlen(arg)) != 0 ||
                writeWithBuffer(fd[1], "\n", 1) != 0) {
                fprintf(2, "buffered write failed\n");
                check_close(close(fd[1]));
                return 1;
            }
        }
        if (flush(fd[1]) != 0) {
            fprintf(2, "flush failed\n");
            check_close(close(fd[1]));
            return 1;
        }

        check_close(close(fd[1]));

        int returnCode = 0;
        if (wait(&returnCode) < 0) {
            fprintf(2, "wait failed\n");
            return 1;
        }
        return returnCode;
    }
}
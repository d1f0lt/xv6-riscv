#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PGSIZE 4096

char buffer[PGSIZE];
int curBufferPos = 0;

int flush(int desc) {
    if (curBufferPos == 0)
        return 0;
    // printf("Скидываю буффер: '%s'\n", buffer);
    if (write(desc, buffer, curBufferPos) != curBufferPos) {
        return 1;
    }
    curBufferPos = 0;
    return 0;
}

int writeWithBuffer(int desc, const void *s, int cnt) {
    if (cnt + curBufferPos >= PGSIZE)
        if (flush(desc) != 0)
            return 1;
    
    memcpy(&buffer[curBufferPos], (const char *)s, cnt);
    curBufferPos += cnt;
    return 0;
}

void check_close(int code) {
    if (code < 0) {
        perror("close failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    // printf("argc = %d\n", argc);
    int fd[2];

    if (pipe(fd) < 0) {
        perror("pipe failed");
        return 1;
    }

    int pid = fork();
    if (pid < 0) {
        perror("fork failed");
        if (close(fd[0]) < 0)
            perror("close failed");
        check_close(close(fd[1]));
        return 1;
    }

    if (pid == 0) {  
        check_close(close(fd[1]));

        if (dup2(fd[0], STDIN_FILENO) != 0) {
            perror("dup");
            check_close(close(fd[0]));
            exit(EXIT_FAILURE);
        }
        check_close(close(fd[0]));

        char readBuffer[PGSIZE];
        int n = 0;
        while ((n = read(STDIN_FILENO, readBuffer, sizeof(readBuffer))) > 0) {
            if (write(STDOUT_FILENO, readBuffer, n) != n) {
                perror("write failed in child");
                return 1;
            }
        }
        if (n < 0) {
            perror("read failed in child");
            return 1;
        }
        return 0;
    } else {
        if (close(fd[0]) < 0) {
            perror("close failed");
            check_close(close(fd[1]));
            return 1;
        }

        for (int i = 1; i < argc; ++i) {
            char *arg = argv[i];
            if (writeWithBuffer(fd[1], arg, strlen(arg)) != 0) {
                fprintf(stderr, "buffered write failed\n");
                check_close(close(fd[1]));
                return 1;
            }

            if (i + 1 < argc) {
                if (writeWithBuffer(fd[1], " ", 1) != 0) {
                    fprintf(stderr, "buffered write failed\n");
                    check_close(close(fd[1]));
                    return 1;
                }
            }
        }

        if (writeWithBuffer(fd[1], "\n", 1) != 0) {
            fprintf(stderr, "buffered write failed\n");
            check_close(close(fd[1]));
            return 1;
        }

        if (flush(fd[1]) != 0) {
            fprintf(stderr, "flush failed\n");
            check_close(close(fd[1]));
            return 1;
        }

        check_close(close(fd[1]));

        int returnCode = 0;
        if (wait(&returnCode) < 0) {
            perror("wait failed");
            return 1;
        }
        return returnCode;
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PGSIZE 4096

char buffer[PGSIZE];
int curBufferPos = 0;

void flush(int desc) {
    if (curBufferPos == 0) return;
    // printf("Скидываю буффер: '%s'\n", buffer);
    if (write(desc, buffer, curBufferPos) != curBufferPos) {
        perror("Error when writing to pipe");
        close(desc);
        exit(EXIT_FAILURE);
    } 
    curBufferPos = 0;
}

void writeWithBuffer(int desc, const void *s, int cnt) {
    if (cnt + curBufferPos >= PGSIZE)
        flush(desc);
    
    memcpy(&buffer[curBufferPos], (const char *)s, cnt);
    curBufferPos += cnt;
}

int main(int argc, char *argv[]) {
    // printf("argc = %d\n", argc);
    int fd[2];

    pipe(fd);

    int pid = fork();
    if (pid < 0) return 1;

    if (pid == 0) {  
        close(fd[1]);

        if (dup2(fd[0], STDIN_FILENO) != 0) {
            perror("dup");
            return 1;
        }
        close(fd[0]);

        execlp("wc", "wc", NULL);
        perror("Error when trying to use wc");
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
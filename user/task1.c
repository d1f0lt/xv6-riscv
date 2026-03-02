#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

const int DELAY = 5;

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "a") != 0 && strcmp(argv[1], "b") != 0))
        return 1;

    int pid = fork();
    if (pid < 0) {
        return 1;
    } else if (pid == 0) {
        pause(DELAY * 10);
        return 1;
    } else {
        printf("My pid: %d\nChild's pid: %d\n", getpid(), pid);

        int returnCode = 0;
        if (strcmp(argv[1], "a") == 0) {
            wait(&returnCode);
            printf("Child's pid: %d\n Child's return code: %d\n", pid, returnCode);
        } else {
            kill(pid);
            if (wait(&returnCode) == -1)
                return 1;
            printf("Victim's pid: %d\nVictim's code: %d\n", pid, returnCode);
        }
    }
}
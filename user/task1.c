#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

const int DELAY = 5;

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "a") != 0 && strcmp(argv[1], "b") != 0))
        return 1;

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        return 1;
    } else if (pid == 0) {
        pause(DELAY * 10);
        return 1;
    } else {
        printf("My pid: %d\nChild's pid: %d\n", getpid(), pid);

        int returnCode = 0;
        if (strcmp(argv[1], "a") == 0) {
            int childPid = wait(&returnCode);
            if (childPid < 0) {
                fprintf(2, "wait failed\n");
                return 1;
            }
            printf("Child's pid: %d\n Child's return code: %d\n", childPid, returnCode);
        } else {
            if (kill(pid) < 0) {
                fprintf(2, "kill failed");
                return 1;
            }
            int childPid = wait(&returnCode);
            if (childPid < 0) {
                fprintf(2, "wait failed after kill\n");
                return 1;
            }
            printf("Victim's pid: %d\nVictim's code: %d\n", childPid, returnCode);
        }
    }
    return 0;
}
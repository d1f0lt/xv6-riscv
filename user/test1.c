#include "kernel/types.h"
#include "user/user.h"

int
print_args(int mfd, int argc, char **argv) // mfd < 0 if without mutex
{
    int pid = getpid();

    for (int i = 1; i < argc; i++)
    {
        char *s = argv[i];
        for (int j = 0; s[j] != 0; j++)
        {
            if (mfd >= 0)
                if (mutex_lock(mfd) < 0)
                {
                    fprintf(2, "mutex_lock failed\n");
                    return -1;
                }

            printf("%d: arg %d, char '%c'\n", pid, i, s[j]);

            if (mfd >= 0)
            {
                if (mutex_unlock(mfd) < 0)
                {
                    fprintf(2, "mutex_unlock failed\n");
                    return -1;
                }
                // pause(1); 
            }
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int status;
    int pid;
    int mfd;

    if (argc < 2)
    {
        fprintf(2, "at least 1 argument is needed");
        return 1;
    }

    printf("\n--- without mutex ---\n");
    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        return 1;
    }

    if (print_args(-1, argc, argv) < 0)
        return -1;

    if (pid == 0)
        return 0;

    wait(&status);
    if (status != 0)
        return status;
    printf("---------------------\n");

    printf("\n--- with mutex ---\n");
    mfd = mutex();
    if (mfd < 0)
    {
        fprintf(2, "mutex create failed\n");
        return 1;
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        mutex_close(mfd);
        return 1;
    }

    if (print_args(mfd, argc, argv) < 0) {
        mutex_close(mfd);
        return 1;
    }

    if (pid == 0)
        return 0;

    wait(&status);
    if (status != 0)
    {
        mutex_close(mfd);
        return status;
    }

    if (mutex_close(mfd) < 0)
    {
        fprintf(2, "mutex_close failed\n");
        return 1;
    }

    printf("------------------\n");

    return 0;
}

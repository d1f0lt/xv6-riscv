#include "kernel/types.h"
#include "user/user.h"

int fails = 0;

void
check_eq(char *name, int got, int want)
{
    if (got == want)
        printf("PASS: %s (got %d)\n", name, got);
    else
    {
        fprintf(2, "%s (got %d, want %d)\n", name, got, want);
        fails++;
    }
}

void
test_read_write_mutex(void)
{
    int mfd;
    char b = 'X';
    char out = 0;

    printf("\n[1] read/write\n");
    mfd = mutex();
    if (mfd < 0)
    {
        fprintf(2, "mutex create\n");
        fails++;
        return;
    }

    check_eq("read", read(mfd, &out, 1), -1);
    check_eq("write", write(mfd, &b, 1), -1);
}

void
test_close_locked_by_owner(void)
{
    int mfd;

    printf("\n[2a] close locked mutex by owner\n");
    mfd = mutex();
    if (mfd < 0)
    {
        fprintf(2, "mutex create\n");
        fails++;
        return;
    }

    check_eq("mutex_lock(owner)", mutex_lock(mfd), 0);
    check_eq("mutex_close(owner while locked)", mutex_close(mfd), 0);
    check_eq("mutex_unlock(closed fd)", mutex_unlock(mfd), -1);
}

void test_close_locked_by_other_process(void)
{
    int mfd;
    int pid;
    int status;

    printf("\n[2b] close locked mutex by another process\n");
    mfd = mutex();
    if (mfd < 0)
    {
        fprintf(2, "mutex create\n");
        fails++;
        return;
    }

    if (mutex_lock(mfd) != 0)
    {
        fprintf(2, "parent lock\n");
        fails++;
        mutex_close(mfd);
        return;
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork\n");
        fails++;
        mutex_unlock(mfd);
        mutex_close(mfd);
        return;
    }

    if (pid == 0)
    {
        int rc = mutex_close(mfd);
        exit(rc == 0 ? 0 : 1);
    }

    wait(&status);
    check_eq("child close inherited fd", status, 0);
    check_eq("parent still owns lock", mutex_unlock(mfd), 0);
    check_eq("parent close", mutex_close(mfd), 0);
}

void test_exit_with_unclosed_mutexes(void)
{
    int pid;
    int status;

    printf("\n[3a] process exits with unclosed mutexes\n");
    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork\n");
        fails++;
        return;
    }

    if (pid == 0)
    {
        int a = mutex();
        int b = mutex();
        if (a < 0 || b < 0)
            exit(1);
        exit(0);
    }

    wait(&status);
    check_eq("child exit with 2 unclosed mutexes", status, 0);
}

void
test_exit_with_mutex_locked_by_other(void)
{
    int mfd;
    int pid;
    int status;

    printf("\n[3b] process exits with mutex fd locked by another process\n");
    mfd = mutex();
    if (mfd < 0)
    {
        fprintf(2, "mutex create\n");
        fails++;
        return;
    }

    if (mutex_lock(mfd) != 0)
    {
        fprintf(2, "parent lock\n");
        fails++;
        mutex_close(mfd);
        return;
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork\n");
        fails++;
        mutex_unlock(mfd);
        mutex_close(mfd);
        return;
    }

    if (pid == 0)
    {
        exit(0);
    }

    wait(&status);
    check_eq("child exit with inherited locked mutex fd", status, 0);
    check_eq("parent unlock after child exit", mutex_unlock(mfd), 0);
    check_eq("parent close", mutex_close(mfd), 0);
}

void
test_unlock_by_other_process(void)
{
    int mfd;
    int pid;
    int status;

    printf("\n[4] unlock mutex held by another process\n");
    mfd = mutex();
    if (mfd < 0)
    {
        fprintf(2, "mutex create\n");
        fails++;
        return;
    }

    if (mutex_lock(mfd) != 0)
    {
        fprintf(2, "parent lock\n");
        fails++;
        mutex_close(mfd);
        return;
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork\n");
        fails++;
        mutex_unlock(mfd);
        mutex_close(mfd);
        return;
    }

    if (pid == 0)
    {
        int rc = mutex_unlock(mfd);
        exit(rc == -1 ? 0 : 1);
    }

    wait(&status);
    check_eq("child cannot unlock foreign mutex", status, 0);
    check_eq("parent unlock", mutex_unlock(mfd), 0);
    check_eq("parent close", mutex_close(mfd), 0);
}

int main(void)
{
    printf("==== mutex edge-case tests ====\n");

    test_read_write_mutex();
    test_close_locked_by_owner();
    test_unlock_by_other_process();
    test_close_locked_by_other_process();
    test_exit_with_unclosed_mutexes();
    test_exit_with_mutex_locked_by_other();

    if (fails == 0)
    {
        printf("\nALL TESTS PASSED\n");
        return 0;
    }

    printf("\nTOTAL FAILURES: %d\n", fails);
    return 1;
}

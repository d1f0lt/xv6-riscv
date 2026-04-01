#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

struct locks
{
    struct spinlock nullstat;
    struct spinlock urandom;
} lock;

uint64 nullstat = 0;
uint64 seed = 857241543264532061ULL;

int min(int arg1, int arg2)
{
    if (arg1 > arg2)
        return arg2;
    return arg1;
}

uint64 get_nullstat()
{
    acquire(&lock.nullstat);
    uint64 copy = nullstat;
    release(&lock.nullstat);
    return copy;
}

void add_to_nullstat(int val)
{
    acquire(&lock.nullstat);
    nullstat += val;
    release(&lock.nullstat);
}

uint64 get_next()
{
    static const uint64 a = 16807;
    acquire(&lock.urandom);
    seed = seed * a + 1;
    uint64 copy = seed;
    release(&lock.urandom);
    return copy;
}

void set_seed(uint64 new_val)
{
    acquire(&lock.urandom);
    seed = new_val;
    release(&lock.urandom);
}

int write_u64_to_dst(int user_dst, uint64 dst, int n, uint64 val)
{
    if (n != sizeof(uint64))
        return -1;
    if (either_copyout(user_dst, dst, &val, sizeof(val)) == -1)
        return -1;
    return sizeof(uint64);
}

int driverread(int user_dst, uint64 dst, int n, int major, int minor)
{
    if (major != MY_DRIVER || n < 0)
        return -1;

    switch (minor)
    {
    case NULL_MINOR:
        return 0;
    case ZERO_MINOR:
    {
        char zeros[64];
        memset(zeros, 0, sizeof(zeros));

        int read = 0;
        while (read < n)
        {
            int cnt = min((int)sizeof(zeros), n - read);
            if (either_copyout(user_dst, dst + read, zeros, cnt) == -1)
                break;
            read += cnt;
        }
        if (read == 0 && n != 0)
            return -1;
        return read;
    }
    case NULLSTAT_MINOR:
        return write_u64_to_dst(user_dst, dst, n, get_nullstat());
    case URANDOM_MINOR:
    {
        int read = 0;
        while (read < n)
        {
            uint64 rnd = get_next();
            for (int i = 0; i < (int)sizeof(uint64) && read < n; i++)
            {
                char byte = (char)(rnd & 0xFF);
                if (either_copyout(user_dst, dst + read, &byte, 1) == -1)
                    return read == 0 ? -1 : read;
                rnd >>= 8;
                read++;
            }
        }
        return read;
    }

    default:
        return -1;
    }
}

int driverwrite(int user_src, uint64 src, int n, int major, int minor)
{
    if (major != MY_DRIVER || n < 0)
        return -1;

    switch (minor)
    {
    case NULL_MINOR:
        return n;
    case ZERO_MINOR: 
        return -1;
    case NULLSTAT_MINOR:
        add_to_nullstat(n);
        return n;
    case URANDOM_MINOR: 
        if (n != sizeof(uint64))
            return -1;
        uint64 new_seed;
        if (either_copyin(&new_seed, user_src, src, sizeof(new_seed)) == -1)
            return -1;
        set_seed(new_seed);
        return n;

    default:
        return -1;
    }
}

void mydriverinit(void)
{
    initlock(&lock.nullstat, "nullstat");
    initlock(&lock.urandom, "urandom");
    // connect read and write system calls
    // to consoleread and consolewrite.
    devsw[MY_DRIVER].read = driverread;
    devsw[MY_DRIVER].write = driverwrite;
}
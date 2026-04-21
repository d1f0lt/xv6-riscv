#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"

void print_flags(uint64 pte)
{
    printf(" %c%c%c%c%c%c%c",
           (pte & PTE_R) ? 'R' : '_',
           (pte & PTE_W) ? 'W' : '_',
           (pte & PTE_X) ? 'X' : '_',
           (pte & PTE_U) ? 'U' : '_',
           (pte & PTE_G) ? 'G' : '_',
           (pte & PTE_A) ? 'A' : '_',
           (pte & PTE_D) ? 'D' : '_');
}

void print_format_index(int ind)
{
    if (ind < 10) 
        printf("00%d", ind);
    else if (ind < 100)
        printf("0%d", ind);
    else 
        printf("%d", ind);
}


void print_recursively(pagetable_t pagetable, int depth, uint64 va_prefix, uint64 usersz)
{
    int max_depth = 2;
    int level = max_depth - depth;

    for (int i = 0; i < 512; i++)
    {
        pte_t pte = pagetable[i];
        if ((pte & PTE_V) == 0)
            continue;

        uint64 next_va_prefix = va_prefix | ((uint64)i << PXSHIFT(level));

        for (int j = 0; j < depth; j++)
            printf("         ");

        printf("0x");
        print_format_index(i);
        printf(" -> 0x%lx", PTE2PA(pte));

        if (depth == max_depth)
        {
            print_flags(pte);
            printf(" va=0x%lx", next_va_prefix);
            printf("\n");
        }
        else
        {
            printf("\n");
            print_recursively((pagetable_t)PTE2PA(pte), depth + 1, next_va_prefix, usersz);
        }
    }
}

uint64 sys_printPageTable(void)
{
    struct proc *p = myproc();
    if (p == 0)
    {
        printf("myproc() error\n");
        return -1;
    }

    pagetable_t pagetable = p->pagetable;
    printf("PAGETABLE 0x%lx (sz=0x%lx)\n", (uint64)pagetable, p->sz);
    print_recursively(pagetable, 0, 0, p->sz);

    return 0;
}
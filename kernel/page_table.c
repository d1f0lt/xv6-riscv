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

void print_recursively(pagetable_t pagetable, int depth)
{
    int max_depth = 2;
    for (int i = 1; i <= 512; i++)
    {
        pte_t pte = pagetable[i];
        if ((pte & PTE_V) == 0)
            continue;

        for (int j = 0; j < depth; j++)
            printf("         ");

        printf("0x");
        print_format_index(i);
        printf(" -> 0x%lx", PTE2PA(pte));

        if (depth == max_depth)
        {
            print_flags(pte);
            printf("\n");
        }
        else
        {
            printf("\n");
            print_recursively((pagetable_t)PTE2PA(pte), depth + 1);
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
    printf("PAGETABLE 0x%lx\n", (uint64)pagetable);
    print_recursively(pagetable, 0);

    return 0;
}
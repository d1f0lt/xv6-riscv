#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "user/user.h"

int global_var = 42;

void print_title(const char *title)
{
    printf("\n");
    printf("=====================================================\n");
    printf("%s\n", title);
    printf("=====================================================\n");
}

int get_flag(void *addr, int len, int flag)
{
    return checkPageFlags(addr, len, flag);
}

void print_ad(const char *label, int a, int d)
{
    printf("%s: A=%d D=%d\n", label, a, d);
}

void print_info(const char *place, const char *action,
                int before_a, int before_d,
                int after_a, int after_d)
{
    printf("[%s] было A=%d D=%d | действие: %s | стало A=%d D=%d\n",
           place, before_a, before_d, action, after_a, after_d);
}

void my_read(void *addr)
{
    volatile uchar x = *(volatile uchar *)addr;
    (void)x;
}

void my_write(void *addr)
{
    volatile uchar *p = (volatile uchar *)addr;
    *p = *p ^ 1;
    *p = *p ^ 1;
}

void ad_test(const char *place, void *addr, int len)
{
    int before_a = get_flag(addr, len, PTE_A);
    int before_d = get_flag(addr, len, PTE_D);
    print_ad("Изначально", before_a, before_d);

    if (clearPageFlags(addr, len, PTE_A | PTE_D) < 0)
    {
        printf("[%s] clearPageFlags failed\n", place);
        return;
    }
    int clear_a = get_flag(addr, len, PTE_A);
    int clear_d = get_flag(addr, len, PTE_D);
    print_info(place, "clear A|D", before_a, before_d, clear_a, clear_d);

    my_read(addr);
    int read_a = get_flag(addr, len, PTE_A);
    int read_d = get_flag(addr, len, PTE_D);
    print_info(place, "read", clear_a, clear_d, read_a, read_d);

    my_write(addr);
    int write_a = get_flag(addr, len, PTE_A);
    int write_d = get_flag(addr, len, PTE_D);
    print_info(place, "write", read_a, read_d, write_a, write_d);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int stack_var = 123;
    int stack_array[10];
    int stack_sum = 0;
    for (int i = 0; i < 10; i++)
    {
        stack_array[i] = i * 10;
        stack_sum += stack_array[i];
    }

    int heap_size = 10 * PGSIZE;
    int *heap_array = (int *)malloc(heap_size);
    if (heap_array == 0)
    {
        printf("malloc failed\n");
        return -1;
    }

    for (int i = 0; i < heap_size / (int)sizeof(int); i++)
        heap_array[i] = i;

    print_title("Начало");
    printf("data: 0x%lx\n", (uint64)&global_var);
    printf("heap: 0x%lx .. 0x%lx (size=%d)\n",
           (uint64)heap_array, (uint64)heap_array + heap_size - 1, heap_size);
    printf("stack: 0x%lx\n", (uint64)&stack_var);
    printf("stack_array sum: %d\n", stack_sum);
    printPageTable();

    print_title("Изменения AD");
    ad_test("gloabl", &global_var, sizeof(global_var));
    ad_test("heap", heap_array, sizeof(*heap_array));
    ad_test("stack(не работает, из-за постоянной работы со стеком)", &stack_var, sizeof(stack_var));

    print_title("Теперь");
    printPageTable();

    print_title("После free");
    free(heap_array);
    printPageTable();
}

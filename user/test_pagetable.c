#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int global_var = 321;

int main(int argc, char *argv[])
{
    int stack_var = 123;
    int stack_array[10];
    int stack_sum = 0; // иначе пишет, что stack_array не используется :(
    for (int i = 0; i < 10; i++)
    {
        stack_array[i] = i * 10;
        stack_sum += stack_array[i];
    }

    printf("\n");
    printf("=====================================================\n");
    printf("Start\n");
    printf("=====================================================\n");
    printf("Global var=%d, stack_var=%d, stack_array sum=%d\n", global_var, stack_var, stack_sum);
    printPageTable();

    printf("\n");
    printf("=====================================================\n");
    printf("Malloc\n");
    printf("=====================================================\n");
    int size = 10 * 4096;
    int *heap_array = (int *)malloc(size);
    if (heap_array == 0)
    {
        printf("malloc failed\n");
        return -1;
    }
    printf("Addr: 0x%lx\n", (uint64)heap_array);
    printPageTable();

    printf("\n");
    printf("=====================================================\n");
    printf("Clear A/D\n");
    printf("=====================================================\n");
    int flags_mask = (1 << 6) | (1 << 7); 
    if (clearPageFlags(heap_array, size, flags_mask) < 0)
    {
        printf("clearPageFlags failed\n");
        return -1;
    }
    printPageTable();

    printf("\n");
    printf("=====================================================\n");
    printf("Read\n");
    printf("=====================================================\n");
    int sum = 0;
    int count = size / sizeof(int);
    for (int i = 0; i < count; i++)
    {
        sum += heap_array[i];
    }
    int check_result = checkPageFlags(heap_array, size, (1 << 6));
    printf("A flag: %d\n", check_result);
    printPageTable();

    printf("\n");
    printf("=====================================================\n");
    printf("Write\n");
    printf("=====================================================\n");
    for (int i = 0; i < count; i++)
    {
        heap_array[i] = i * 2;
    }
    int check_a = checkPageFlags(heap_array, size, (1 << 6));
    int check_d = checkPageFlags(heap_array, size, (1 << 7));
    printf("A flag: %d, D flag: %d\n", check_a, check_d);
    printPageTable();

    printf("\n");
    printf("=====================================================\n");
    printf("Free\n");
    printf("=====================================================\n");
    free(heap_array);
    printPageTable();

    printf("\n");
    printf("=====================================================\n");
    printf("Stack\n");
    printf("=====================================================\n");
    uint64 stack_addr = (uint64)&stack_var;
    printf("Stack var at 0x%lx\n", stack_addr);

    int stack_check = checkPageFlags(&stack_var, sizeof(int), (1 << 6) | (1 << 7));
    printf("A or D flag: %d\n", stack_check);

    if (clearPageFlags(&stack_var, sizeof(int), (1 << 6) | (1 << 7)) < 0)
    {
        printf("clearPageFlags on stack failed\n");
    }
    else
    {
        printf("Cleared A/D on stack variable\n");

        stack_var = 999;
        int check_after = checkPageFlags(&stack_var, sizeof(int), (1 << 6) | (1 << 7));
        printf("A or D flag after clear: %d\n", check_after);
    }

    printPageTable();

    printf("\n");
    printf("=====================================================\n");
    printf("End\n");
    printf("=====================================================\n");

    return 0;
}

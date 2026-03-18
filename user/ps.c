#include "kernel/types.h"
#include "kernel/procstate.h"
#include "user/user.h"
#include "kernel/procinfo.h"

const char *get_state_name(enum procstate state)
{
    switch (state)
    {
    case UNUSED:
        return "UNUSED";
    case USED:
        return "USED";
    case SLEEPING:
        return "SLEEPING";
    case RUNNABLE:
        return "RUNNABLE";
    case RUNNING:
        return "RUNNING";
    case ZOMBIE:
        return "ZOMBIE";
    default:
        return "UNKNOWN";
    }
}

int main(int argc, char *argv[])
{
    int lim = 1;
    struct procinfo *plist = 0;

    int nproc;
    const char *state;

    do
    {
        struct procinfo *new_plist = malloc(lim * sizeof(struct procinfo));

        if (new_plist == 0)
        {
            fprintf(2, "malloc failed\n");
            if (plist) free(plist);
            return 1;
        }

        if (plist)
            free(plist);
        plist = new_plist;

        nproc = ps_listinfo(plist, lim);

        if (nproc < -1)
        {
            fprintf(2, "ps_listinfo failed\n");
            if (plist) free(plist);
            return 1;
        }
        lim *= 2;
    } while (nproc == -1);

    printf("N PID PPID STATE NAME PARENT\n");
    printf("-----------------------------\n");

    for (int i = 0; i < nproc; i++)
    {
        state = get_state_name(plist[i].state);

         printf("%d %d %d %s %s %s\n",
             i + 1,
             (int)plist[i].pid,
             (int)plist[i].parent_pid,
             state,
             plist[i].name,
             plist[i].pname);
    }

    free(plist);
}

#include "types.h"
#include "riscv.h"
#include "defs.h"

uint64 sys_ps_listinfo(void)
{
    uint64 plist_addr;
    int lim;

    argaddr(0, &plist_addr);
    argint(1, &lim);

    return copy_procinfo_to_user_space(plist_addr, lim);
}

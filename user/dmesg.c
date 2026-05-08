#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/param.h"
#include "user/user.h"

static char buf[DMSG_PAGES * PGSIZE + 1];

int
main(int argc, char **argv)
{
  int n;

  n = dmesg(buf, sizeof(buf));
  if(n < 0){
    fprintf(2, "dmesg: syscall failed\n");
    return 1;
  }

  if(n > 0)
    write(1, buf, n);
}

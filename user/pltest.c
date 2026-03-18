#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main(void)
{
	struct procinfo big[NPROC];
	struct procinfo small[2];
	int n;

	printf("ps_listinfo test start\n");

	n = ps_listinfo((struct procinfo *)0, 4);
	printf("[null ptr] ps_listinfo(0, 4) -> %d\n", n);

	n = ps_listinfo(small, 2);
	printf("[small buffer] ps_listinfo(small, 2) -> %d\n", n);

	n = ps_listinfo((struct procinfo *)-1, 4);
	printf("[bad addr] ps_listinfo((procinfo*)-1, 4) -> %d\n", n);

	n = ps_listinfo(big, -1);
	printf("[negative lim] ps_listinfo(big, -1) -> %d\n", n);

	printf("ps_listinfo test done\n");
}

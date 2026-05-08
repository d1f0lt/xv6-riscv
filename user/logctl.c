#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void
usage(void)
{
  printf("usage: logctl on|off class... [-t ticks]\n");
  printf("classes: syscall intr proc exec all\n");
}

int
parse_class(const char *s)
{
  if(strcmp(s, "syscall") == 0)
    return LOG_SYSCALL;
  if(strcmp(s, "intr") == 0)
    return LOG_INTERRUPT;
  if(strcmp(s, "proc") == 0)
    return LOG_PROC;
  if(strcmp(s, "exec") == 0)
    return LOG_EXEC;
  if(strcmp(s, "all") == 0)
    return LOG_ALL;
  return 0;
}

int
main(int argc, char **argv)
{
  int enable = 0;
  int mask = 0;
  int duration = 0;

  if(argc < 3)
    usage();

  if(strcmp(argv[1], "on") == 0)
    enable = 1;
  else if(strcmp(argv[1], "off") != 0)
    usage();

  for(int i = 2; i < argc; i++){
    if(strcmp(argv[i], "-t") == 0){
      if(i + 1 >= argc)
        usage();
      duration = atoi(argv[i + 1]);
      i++;
      continue;
    }

    int m = parse_class(argv[i]);
    if(m == 0)
      usage();
    mask |= m;
  }

  if(logctl(mask, enable, duration) < 0){
    fprintf(2, "logctl: syscall failed\n");
    return 1;
  }
}

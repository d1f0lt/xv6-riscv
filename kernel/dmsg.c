#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define DMSG_CAPACITY (DMSG_PAGES * PGSIZE)

static struct dmsg_buffer {
  struct spinlock lock;
  char buf[DMSG_CAPACITY];
  uint head;
  uint tail;
  uint size;
} dmsg;

static struct {
  struct spinlock lock;
  int mask;
  uint until;
} logstate;

void
get_ticks(uint *out)
{
  acquire(&tickslock);
  *out = ticks;
  release(&tickslock);
}

void
dmsg_putc_locked(char c)
{
  if(dmsg.size == DMSG_CAPACITY){
    dmsg.tail = (dmsg.tail + 1) % DMSG_CAPACITY;
    dmsg.size--;
  }

  dmsg.buf[dmsg.head] = c;
  dmsg.head = (dmsg.head + 1) % DMSG_CAPACITY;
  dmsg.size++;
}

void
dmsg_putc(char c)
{
  acquire(&dmsg.lock);
  dmsg_putc_locked(c);
  release(&dmsg.lock);
}

static char digits[] = "0123456789abcdef";

static void
printint(long long xx, int base, int sign)
{
  char buf[20];
  int i;
  unsigned long long x;

  if(sign && (sign = (xx < 0)))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    dmsg_putc_locked(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  dmsg_putc_locked('0');
  dmsg_putc_locked('x');
  for(i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    dmsg_putc_locked(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

void
pr_msg(char *fmt, ...)
{
  va_list ap;
  int i, cx, c0, c1, c2;
  char *s;
  uint ticks;

  get_ticks(&ticks);

  acquire(&dmsg.lock);

  dmsg_putc_locked('[');
  printint(ticks, 10, 0);
  dmsg_putc_locked(']');
  dmsg_putc_locked(' ');

  va_start(ap, fmt);
  for(i = 0; (cx = fmt[i] & 0xff) != 0; i++){
    if(cx != '%'){
      dmsg_putc_locked(cx);
      continue;
    }
    i++;
    c0 = fmt[i+0] & 0xff;
    c1 = c2 = 0;
    if(c0) c1 = fmt[i+1] & 0xff;
    if(c1) c2 = fmt[i+2] & 0xff;
    if(c0 == 'd'){
      printint(va_arg(ap, int), 10, 1);
    } else if(c0 == 'l' && c1 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    } else if(c0 == 'u'){
      printint(va_arg(ap, uint32), 10, 0);
    } else if(c0 == 'l' && c1 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 2;
    } else if(c0 == 'x'){
      printint(va_arg(ap, uint32), 16, 0);
    } else if(c0 == 'l' && c1 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    } else if(c0 == 'p'){
      printptr(va_arg(ap, uint64));
    } else if(c0 == 'c'){
      dmsg_putc_locked(va_arg(ap, uint));
    } else if(c0 == 's'){
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        dmsg_putc_locked(*s);
    } else if(c0 == '%'){
      dmsg_putc_locked('%');
    } else if(c0 == 0){
      break;
    } else {
      // Print unknown % sequence to draw attention.
      dmsg_putc_locked('%');
      dmsg_putc_locked(c0);
    }
  }
  va_end(ap);

  dmsg_putc_locked('\n');

  release(&dmsg.lock);
}

int
dmsg_copyout(pagetable_t pagetable, uint64 dst, int max)
{
  uint start;
  uint available;
  uint cap = DMSG_CAPACITY;
  int n;

  if(max <= 0)
    return -1;

  acquire(&dmsg.lock);

  start = dmsg.tail;
  available = dmsg.size;

  if(available > cap)
    available = cap;

  n = available > max - 1 ? max - 1 : available;

  for(int i = 0; i < n; i++){
    char c = dmsg.buf[(start + i) % cap];
    if(copyout(pagetable, dst + i, &c, 1) < 0){
      release(&dmsg.lock);
      return -1;
    }
  }

  {
    char zero = 0;
    if(copyout(pagetable, dst + n, &zero, 1) < 0){
      release(&dmsg.lock);
      return -1;
    }
  }

  release(&dmsg.lock);
  return n;
}

int
check_log_enabled(int mask)
{
  uint now = 0;
  int enabled;

  if(mask == 0)
    return 0;

  get_ticks(&now);

  acquire(&logstate.lock);
  if(logstate.until != 0 && now >= logstate.until){
    logstate.mask = 0;
    logstate.until = 0;
  }
  enabled = (logstate.mask & mask) != 0;
  release(&logstate.lock);

  return enabled;
}

void
logctl(int mask, int enable, int duration)
{
  uint now = 0;

  if(duration > 0)
    get_ticks(&now);

  acquire(&logstate.lock);
  if(enable)
    logstate.mask |= mask;
  else
    logstate.mask &= ~mask;

  if(enable && duration > 0)
    logstate.until = now + duration;
  else if(!enable && logstate.mask == 0)
    logstate.until = 0;

  release(&logstate.lock);
}

void
dmsg_init(void)
{
  initlock(&dmsg.lock, "dmsg");
  initlock(&logstate.lock, "log");

  dmsg.head = 0;
  dmsg.tail = 0;
  dmsg.size = 0;

  logstate.mask = 0;
  logstate.until = 0;
}

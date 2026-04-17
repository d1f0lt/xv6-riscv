#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

#define Reg(reg) ((volatile uint32 *)(reg))
#define ReadReg(reg) (*(Reg(reg)))

static struct spinlock rtc_lock;

static uint32
rtc_read_low(void)
{
  return ReadReg(RTC0_LOW);
}

static uint32
rtc_read_high(void)
{
  return ReadReg(RTC0_HIGH);
}

void
rtcinit(void)
{
  initlock(&rtc_lock, "rtc");
}

uint64
rtctime(void)
{
  acquire(&rtc_lock);
  uint32 low = rtc_read_low();
  uint32 high = rtc_read_high();
  release(&rtc_lock);

  return ((uint64)high << 32) | low;
}

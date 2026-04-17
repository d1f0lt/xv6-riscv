#include "kernel/types.h"
#include "user/user.h"

int
is_leap(int year)
{
    if (year % 400 == 0)
        return 1;
    if (year % 100 == 0)
        return 0;
    return (year % 4) == 0;
}

int
days_in_month(int year, int month)
{
    static int month_days[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && is_leap(year))
        return 29;
    return month_days[month - 1];
}

void
split_hms(uint64 secs_in_day, int *hour, int *minute, int *second,
          uint64 sec_per_hour, uint64 sec_per_min)
{
    *hour = (int)(secs_in_day / sec_per_hour);
    secs_in_day %= sec_per_hour;
    *minute = (int)(secs_in_day / sec_per_min);
    *second = (int)(secs_in_day % sec_per_min);
}

void
split_ymd(uint64 days, int *year, int *month, int *day)
{
    int y, m;

    y = 1970;
    while (1)
    {
        int ydays;

        ydays = is_leap(y) ? 366 : 365;
        if (days < (uint64)ydays)
            break;
        days -= (uint64)ydays;
        y++;
    }

    m = 1;
    while (m <= 12)
    {
        int mdays;

        mdays = days_in_month(y, m);
        if (days < (uint64)mdays)
            break;
        days -= (uint64)mdays;
        m++;
    }

    *year = y;
    *month = m;
    *day = (int)days + 1;
}

void
print_2d(int value)
{
    if (value < 10)
        printf("0");
    printf("%d", value);
}

void
print_frac_ns(uint32 value)
{
    uint32 div;

    div = 1e8;
    while (div > 0)
    {
        printf("%d", (int)(value / div));
        value %= div;
        div /= 10;
    }
}

void
print_datetime(int year, int month, int day,
               int hour, int minute, int second, uint32 frac_ns)
{
    printf("%d-", year);
    print_2d(month);
    printf("-");
    print_2d(day);
    printf(" ");
    print_2d(hour);
    printf(":");
    print_2d(minute);
    printf(":");
    print_2d(second);
    printf(".");
    print_frac_ns(frac_ns);
    printf("\n");
}

int
main(int argc, char *argv[])
{
    const uint64 ns_per_sec = (uint64)1e9, sec_per_min = 60ULL,
                 sec_per_hour = 60ULL * sec_per_min,
                 sec_per_day = 24ULL * sec_per_hour;
    uint64 ns, secs, days, secs_in_day;
    int year, month, day, hour, minute, second;
    uint32 frac_ns;

    (void)argc;
    (void)argv;

    ns = rtctime();
    secs = ns / ns_per_sec;
    frac_ns = (uint32)(ns % ns_per_sec);
    days = secs / sec_per_day;
    secs_in_day = secs % sec_per_day;

    split_hms(secs_in_day, &hour, &minute, &second, sec_per_hour, sec_per_min);
    split_ymd(days, &year, &month, &day);
    print_datetime(year, month, day, hour, minute, second, frac_ns);
}

#include "kernel/types.h"
#include "user/user.h"

int64
floor_div(int64 a, int64 b)
{
    int64 q;
    int64 r;

    q = a / b;
    r = a % b;
    if (r < 0)
        q -= 1;
    return q;
}

int64
floor_mod(int64 a, int64 b)
{
    int64 r;

    r = a % b;
    if (r < 0)
        r += b;
    return r;
}

void
split_hms(int64 secs_in_day, int *hour, int *minute, int *second,
          int64 sec_per_hour, int64 sec_per_min)
{
    *hour = (int)(secs_in_day / sec_per_hour);
    secs_in_day %= sec_per_hour;
    *minute = (int)(secs_in_day / sec_per_min);
    *second = (int)(secs_in_day % sec_per_min);
}

void
split_ymd(int64 days, int *year, int *month, int *day)
{
    int64 z;
    int64 era;
    int64 doe;
    int64 yoe;
    int64 y;
    int64 doy;
    int64 mp;

    z = days + 719468;
    era = (z >= 0 ? z : z - 146096) / 146097;
    doe = z - era * 146097;
    yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = yoe + era * 400;
    doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    mp = (5 * doy + 2) / 153;

    *day = (int)(doy - (153 * mp + 2) / 5 + 1);
    *month = (int)(mp + (mp < 10 ? 3 : -9));
    y += (*month <= 2);
    *year = (int)y;
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

    div = 100000000;
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
    const int64 ns_per_sec = 1000000000LL, sec_per_min = 60LL,
                sec_per_hour = 60LL * sec_per_min,
                sec_per_day = 24LL * sec_per_hour;
    int64 ns, secs, days, secs_in_day;
    int year, month, day, hour, minute, second;
    uint32 frac_ns;

    (void)argc;
    (void)argv;

    ns = (int64)rtctime();
    secs = floor_div(ns, ns_per_sec);
    frac_ns = (uint32)floor_mod(ns, ns_per_sec);
    days = floor_div(secs, sec_per_day);
    secs_in_day = floor_mod(secs, sec_per_day);

    split_hms(secs_in_day, &hour, &minute, &second, sec_per_hour, sec_per_min);
    split_ymd(days, &year, &month, &day);
    print_datetime(year, month, day, hour, minute, second, frac_ns);
}

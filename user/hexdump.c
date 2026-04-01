#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char int_to_hex(int v)
{
    if (v < 10)
        return '0' + v;
    return 'A' + (v - 10);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "hexdump: format error\n");
        return 1;
    }

    int count = atoi(argv[1]);
    int fd = open(argv[2], O_RDONLY);

    if (fd < 0)
    {
        fprintf(2, "hexdump: cannot open %s\n", argv[2]);
        return 1;
    }

    for (int i = 0; i < count; i++)
    {
        char c;
        int n = read(fd, &c, 1);
        if (n < 0)
        {
            fprintf(2, "hexdump: read error\n");
            close(fd);
            return 1;
        }
        if (n == 0)
            break;

        char out[3];
        out[0] = int_to_hex((c >> 4) & 0xF);
        out[1] = int_to_hex(c & 0xF);
        out[2] = (i + 1 == count) ? '\n' : ' ';
        if (write(1, out, sizeof(out)) != sizeof(out))
        {
            fprintf(2, "hexdump: write error\n");
            close(fd);
            return 1;
        }
    }

    close(fd);
}

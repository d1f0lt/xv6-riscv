#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
char_to_int(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(2, "hexwrite: format error\n");
		return 1;
	}

	char *hex = argv[1];
	int hexlen = strlen(hex);
	if (hexlen == 0 || (hexlen % 2) != 0)
	{
		fprintf(2, "hexwrite: invalid hex length\n");
		return 1;
	}

	int bytelen = hexlen / 2;
	char *buf = malloc(bytelen);
	if (buf == 0)
	{
		fprintf(2, "malloc: failed\n");
		return 1;
	}

	for (int i = 0; i < bytelen; i++)
	{
		int hi = char_to_int(hex[2 * i]);
		int lo = char_to_int(hex[2 * i + 1]);
		if (hi == -1 || lo == -1)
		{
			fprintf(2, "hexwrite: invalid hex\n");
			free(buf);
			return 1;
		}
		buf[i] = (char)((hi << 4) | lo);
	}

	int fd = open(argv[2], O_WRONLY);
	if (fd < 0)
	{
		fprintf(2, "hexwrite: cannot open %s\n", argv[2]);
		free(buf);
		return 1;
	}
	
	int written = 0;
	while (written < bytelen)
	{
		int n = write(fd, buf + written, bytelen - written);
		if (n <= 0)
		{
			fprintf(2, "Write error\n");
			close(fd);
			free(buf);
			return 1;
		}
		written += n;
	}

	close(fd);
	free(buf);
}

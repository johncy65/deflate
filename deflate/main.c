#include <stdio.h>

#include "inflate.h"

int main(int argc, char *argv[])
{
	FILE *file;
	int size;
	unsigned char *buffer;
	Inflate *inflate;

	file = fopen("test.gz", "rb");
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = (unsigned char*)malloc(size);
	fread(buffer, 1, size, file);

	inflate = inflate_new(buffer + 15, size - 15);
	inflate_read(inflate);
	return 0;
}

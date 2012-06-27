#include <stdio.h>

#include "inflate.h"
#include "gzip.h"

int main(int argc, char *argv[])
{
	FILE *file;
	int size;
	unsigned char *buffer;
	Inflate *inflate;
	int gzip_len;

	if(argc < 2) {
		fprintf(stderr, "No filename specified\n");
		exit(1);
	}

	if(fopen_s(&file, argv[1], "rb") != 0) {
		fprintf(stderr, "Error opening file %s\n", argv[1]);
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = (unsigned char*)malloc(size);
	fread(buffer, 1, size, file);

	gzip_len = gzip_header_length(buffer, size);
	inflate = inflate_new(buffer + gzip_len, size - gzip_len);
	inflate_read(inflate);

	return 0;
}

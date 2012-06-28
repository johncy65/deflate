#include <stdio.h>
#include <stdlib.h>

#include "Inflate.hpp"
#include "GZip.hpp"

int main(int argc, char *argv[])
{
	FILE *file;
	int size;
	unsigned char *buffer;
	Inflate *inflate;
	int gzipLen;

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

	gzipLen = GZip::headerLength(buffer, size);
	inflate = new Inflate(buffer + gzipLen, size - gzipLen);
	inflate->read();

	return 0;
}

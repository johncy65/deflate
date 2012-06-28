#include <stdio.h>
#include <stdlib.h>

#include "Inflate.hpp"
#include "GZip.hpp"

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
	if(argc < 2) {
		fprintf(stderr, "No filename specified\n");
		exit(1);
	}

	FILE *file;
	if(fopen_s(&file, argv[1], "rb") != 0) {
		fprintf(stderr, "Error opening file %s\n", argv[1]);
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);

	unsigned char *input = new unsigned char[size];
	fread(input, 1, size, file);
	fclose(file);

	int gzipLen = GZip::headerLength(input, size);
	Inflate *inflate = new Inflate(input + gzipLen, size - gzipLen);

	unsigned char *output = new unsigned char[BUFFER_SIZE];
	while(1) {
		int bytesRead = inflate->read(output, BUFFER_SIZE);
		if(inflate->empty()) {
			break;
		}
	}

	delete[] input;
	delete[] output;

	return 0;
}

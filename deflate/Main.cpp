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

	Reader *reader = new FileReader(file);
	GZip *gzip = new GZip(reader);

	unsigned char *buffer = new unsigned char[BUFFER_SIZE];
	while(1) {
		int bytesRead = gzip->read(buffer, BUFFER_SIZE);
		if(gzip->empty()) {
			break;
		}
	}

	return 0;
}

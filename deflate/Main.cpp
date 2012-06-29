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

	const char *filename = argv[1];
	FILE *file;
	if(fopen_s(&file, filename, "rb") != 0) {
		fprintf(stderr, "Error opening file %s\n", filename);
		exit(1);
	}

	Reader *reader = new FileReader(file);

	try {
		GZip *gzip = new GZip(reader);

		unsigned char *buffer = new unsigned char[BUFFER_SIZE];
		while(1) {
			int bytesRead = gzip->read(buffer, BUFFER_SIZE);

			fwrite(buffer, 1, bytesRead, stdout);

			if(gzip->empty()) {
				break;
			}
		}
	} catch(GZip::InvalidFormatException e) {
		fprintf(stderr, "File %s is not a valid GZip file\n", filename);
	} catch(GZip::ReadException e) {
		fprintf(stderr, "*** Read error at position %i ***\n", e.position());
	}

	return 0;
}

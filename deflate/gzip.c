#include "gzip.h"

#define FTEXT     0x1
#define FHCRC     0x2
#define FEXTRA    0x4
#define FNAME     0x8
#define FCOMMENT  0x10

int gzip_header_length(const unsigned char *buffer, int length)
{
	unsigned char flg;
	short int xlen;
	int idx;
	flg = buffer[3];

	idx = 10;
	if(flg & FEXTRA) {
		xlen = *(short int*)&buffer[idx];
		idx += xlen + 2;
	}

	if(flg & FNAME) {
		while(buffer[idx] != '\0') {
			idx++;
		}
		idx++;
	}

	if(flg & FCOMMENT) {
		while(buffer[idx] != '\0') {
			idx++;
		}
		idx++;
	}

	return idx;
}

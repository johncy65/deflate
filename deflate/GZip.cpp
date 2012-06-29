#include "GZip.hpp"

#include "Reader.hpp"

#define FTEXT     0x1
#define FHCRC     0x2
#define FEXTRA    0x4
#define FNAME     0x8
#define FCOMMENT  0x10

void GZip::readHeader(Reader *reader)
{
	unsigned int flg;

	reader->readBytes(NULL, 3);
	flg = reader->readBits(8);
	reader->readBytes(NULL, 6);
	if(flg & FEXTRA) {
		unsigned int xlen = reader->readBits(16);
		reader->readBytes(NULL, xlen);
	}

	if(flg & FNAME) {
		char c;
		do {
			c = reader->readBits(8);
		} while(c != '\0');
	}

	if(flg & FCOMMENT) {
		char c;
		do {
			c = reader->readBits(8);
		} while(c != '\0');
	}
}

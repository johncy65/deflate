#include "GZip.hpp"

#include "Reader.hpp"
#include "Inflate.hpp"

#define FTEXT     0x1
#define FHCRC     0x2
#define FEXTRA    0x4
#define FNAME     0x8
#define FCOMMENT  0x10

GZip::GZip(Reader *reader)
{
	unsigned int id1;
	unsigned int id2;
	unsigned int cm;
	unsigned int flg;
	unsigned int mtime;
	unsigned int xfl;
	unsigned int os;

	id1 = reader->readBits(8);
	id2 = reader->readBits(8);
	cm = reader->readBits(8);
	flg = reader->readBits(8);
	mtime = reader->readBits(32);
	xfl = reader->readBits(8);
	os = reader->readBits(8);

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

	mInflate = new Inflate(reader);
}

int GZip::read(unsigned char *buffer, int length)
{
	return mInflate->read(buffer, length);
}

bool GZip::empty()
{
	return mInflate->empty();
}
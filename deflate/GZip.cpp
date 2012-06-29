#include "GZip.hpp"

#include "Reader.hpp"
#include "Inflate.hpp"

static const int FText = 0x1;
static const int FHCRC = 0x2;
static const int FExtra = 0x4;
static const int FName = 0x8;
static const int FComment = 0x10;

static const int Id1 = 0x1f;
static const int Id2 = 0x8b;

static const int CompressionMethodDeflate = 0x8;

static const int ReservedFlags = 0x70;

GZip::GZip(Reader &reader)
: mInflate(reader)
{
	unsigned int id1;
	unsigned int id2;
	unsigned int cm;
	unsigned int flg;
	unsigned int mtime;
	unsigned int xfl;
	unsigned int os;

	try {
		id1 = reader.readBits(8);
		if(id1 != Id1) {
			throw InvalidFormatException();
		}

		id2 = reader.readBits(8);
		if(id2 != Id2) {
			throw InvalidFormatException();
		}

		cm = reader.readBits(8);
		if(cm != CompressionMethodDeflate) {
			throw ReadException(reader.position());
		}

		flg = reader.readBits(8);
		if(flg & ReservedFlags) {
			throw ReadException(reader.position());
		}

		mtime = reader.readBits(32);
		xfl = reader.readBits(8);
		os = reader.readBits(8);

		if(flg & FExtra) {
			unsigned int xlen = reader.readBits(16);
			reader.readBytes(NULL, xlen);
		}

		if(flg & FName) {
			char c;
			do {
				c = reader.readBits(8);
			} while(c != '\0');
		}

		if(flg & FComment) {
			char c;
			do {
				c = reader.readBits(8);
			} while(c != '\0');
		}
	} catch(Reader::EndException e) {
		throw ReadException(e.position());
	}
}

int GZip::read(unsigned char *buffer, int length)
{
	try {
		return mInflate.read(buffer, length);
	} catch(Inflate::ReadException e) {
		throw ReadException(e.position());
	}
}

#ifndef GZIP_H
#define GZIP_H

class BitReader;

class GZip {
public:
	static void readHeader(BitReader *reader);
};

#endif
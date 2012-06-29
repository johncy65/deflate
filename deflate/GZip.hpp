#ifndef GZIP_H
#define GZIP_H

class Reader;

class GZip {
public:
	static void readHeader(Reader *reader);
};

#endif
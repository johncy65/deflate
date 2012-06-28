#ifndef GZIP_H
#define GZIP_H

class GZip {
public:
	static int headerLength(const unsigned char *buffer, int length);
};

#endif
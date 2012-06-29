#ifndef GZIP_H
#define GZIP_H

class Reader;
class Inflate;

class GZip {
public:
	GZip(Reader *reader);

	int read(unsigned char *buffer, int length);
	bool empty();

private:
	Inflate *mInflate;	
};

#endif
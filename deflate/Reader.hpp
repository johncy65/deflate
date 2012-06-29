#ifndef READER_H
#define READER_H

#include <stdio.h>

class Reader {
public:
	Reader();
	virtual ~Reader();

	unsigned int readBits(int num);
	int readBytes(unsigned char *buffer, int length);

	void byteSync();
	bool empty();

protected:
	void setBuffer(const unsigned char *buffer, int length);

private:
	void updateCurrent();

	virtual int refillBuffer() = 0;

	const unsigned char *mBuffer;
	int mLength;
	int mPos;
	unsigned char mCurrent;
	int mBit;
};

class BufferReader : public Reader {
public:
	BufferReader(const unsigned char *buffer, int length);

private:

	virtual int refillBuffer();
};

class FileReader : public Reader {
public:
	FileReader(FILE *file);
	virtual ~FileReader();

private:
	virtual int refillBuffer();

	FILE *mFile;
	unsigned char *mMutableBuffer;
};

#endif

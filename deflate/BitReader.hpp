#ifndef BIT_READER_H
#define BIT_READER_H

#include <stdio.h>

class BitReader {
public:
	BitReader();
	virtual ~BitReader();

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

class BitReaderBuffer : public BitReader {
public:
	BitReaderBuffer(const unsigned char *buffer, int length);

private:

	virtual int refillBuffer();
};

class BitReaderFile : public BitReader {
public:
	BitReaderFile(FILE *file);
	virtual ~BitReaderFile();

private:
	virtual int refillBuffer();

	FILE *mFile;
	unsigned char *mMutableBuffer;
};

#endif

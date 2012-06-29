#ifndef BIT_READER_H
#define BIT_READER_H

class BitReader {
public:
	BitReader(const unsigned char *buffer, int length);

	unsigned int readBits(int num);
	int readBytes(unsigned char *buffer, int length);

	void byteSync();
	bool empty();

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

#endif

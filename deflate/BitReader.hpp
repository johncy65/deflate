#ifndef BIT_READER_H
#define BIT_READER_H

class BitReader {
public:
	BitReader(const unsigned char *buffer, int length);

	unsigned int read(int numBits);
	void byteSync();
	const unsigned char *buffer();
	void advance(int length);

private:
	const unsigned char *mBuffer;
	int mLength;
	int mPos;
	unsigned char mCurrent;
	int mBit;
};

#endif
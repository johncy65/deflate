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
	const unsigned char *mBuffer;
	int mLength;
	int mPos;
	unsigned char mCurrent;
	int mBit;
};

#endif
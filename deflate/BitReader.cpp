#include "BitReader.hpp"

BitReader::BitReader(const unsigned char *buffer, int length)
{
	mBuffer = buffer;
	mLength = length;
	mPos = 0;
	mCurrent = mBuffer[mPos];
	mBit = 0;
}

unsigned int BitReader::read(int numBits)
{
	unsigned int result = 0;
	int read = 0;
	while(read < numBits) {
		int left = numBits - read;
		unsigned int newbits = mCurrent;
		int x = 8 - mBit;
		if(x > left) {
			x = left;
		}

		newbits &= ~(0xffffffff << x);
		result |= newbits << read;

		read += x;
		mCurrent >>= x;
		mBit += x;
		if(mBit == 8) {
			mPos++;
			mCurrent = mBuffer[mPos];
			mBit = 0;
		}
	}

	return result;
}

void BitReader::byteSync()
{
	mPos++;
	mCurrent = mBuffer[mPos];
	mBit = 0;
}

const unsigned char *BitReader::buffer()
{
	return &mBuffer[mPos];
}

void BitReader::advance(int length)
{
	mPos += length;
	mCurrent = mBuffer[mPos];
}

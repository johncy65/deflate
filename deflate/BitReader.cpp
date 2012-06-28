#include "BitReader.hpp"

#include <string.h>

BitReader::BitReader(const unsigned char *buffer, int length)
{
	mBuffer = buffer;
	mLength = length;
	mPos = 0;
	mCurrent = mBuffer[mPos];
	mBit = 0;
}

unsigned int BitReader::readBits(int num)
{
	unsigned int result = 0;
	int read = 0;
	while(read < num) {
		int left = num - read;
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
			mBit = 0;

			mPos++;
			if(mPos == mLength) {
				break;
			}

			mCurrent = mBuffer[mPos];
		}
	}

	return result;
}

int BitReader::readBytes(unsigned char *buffer, int length)
{
	int readLength = length;
	if(readLength > mLength - mPos) {
		readLength = mLength - mPos;
	}

	memcpy(buffer, mBuffer + mPos, readLength);
	mPos += readLength;

	return readLength;
}

void BitReader::byteSync()
{
	if(mBit > 0) {
		mPos++;
		mBit = 0;

		if(mPos < mLength) {
			mCurrent = mBuffer[mPos];
		}
	}
}

bool BitReader::empty()
{
	return (mPos == mLength);
}
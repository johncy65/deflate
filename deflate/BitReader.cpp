#include "BitReader.hpp"

#include <string.h>

BitReader::BitReader(const unsigned char *buffer, int length)
{
	mBuffer = buffer;
	mLength = length;
	mPos = 0;
	mBit = 0;

	updateCurrent();
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
			updateCurrent();
		}
	}

	return result;
}

int BitReader::readBytes(unsigned char *buffer, int length)
{
	int bytesRead = 0;

	while(bytesRead < length)
	{
		if(mLength == 0) {
			break;
		}

		int readLength = length;
		if(readLength > mLength - mPos) {
			readLength = mLength - mPos;
		}

		memcpy(buffer, mBuffer + mPos, readLength);
		mPos += readLength;
		bytesRead += readLength;

		updateCurrent();
	}

	return bytesRead;
}

void BitReader::byteSync()
{
	if(mBit > 0) {
		mPos++;
		mBit = 0;

		updateCurrent();
	}
}

bool BitReader::empty()
{
	return (mLength == 0);
}

void BitReader::updateCurrent()
{
	if(mPos >= mLength) {
		mPos = 0;
		mLength = refillBuffer();
	}

	if(mPos < mLength) {
		mCurrent = mBuffer[mPos];
	}
}

BitReaderBuffer::BitReaderBuffer(const unsigned char *buffer, int length)
: BitReader(buffer, length)
{
}

int BitReaderBuffer::refillBuffer()
{
	return 0;
}

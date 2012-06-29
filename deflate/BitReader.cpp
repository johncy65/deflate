#include "BitReader.hpp"

#include <string.h>

BitReader::BitReader()
{
	mBuffer = NULL;
	mLength = 0;
	mPos = 0;
	mBit = 0;
}

BitReader::~BitReader()
{
}

void BitReader::setBuffer(const unsigned char *buffer, int length)
{
	mBuffer = buffer;
	mLength = length;

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

		if(buffer) {
			memcpy(buffer, mBuffer + mPos, readLength);
		}

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
{
	setBuffer(buffer, length);
}

int BitReaderBuffer::refillBuffer()
{
	return 0;
}

#define BUFFER_SIZE 4096
BitReaderFile::BitReaderFile(FILE *file) 
{
	mFile = file;
	mMutableBuffer = new unsigned char[BUFFER_SIZE];
	setBuffer(mMutableBuffer, 0);
}

BitReaderFile::~BitReaderFile()
{
	delete[] mMutableBuffer;
}

int BitReaderFile::refillBuffer()
{
	return fread(mMutableBuffer, 1, BUFFER_SIZE, mFile);
}
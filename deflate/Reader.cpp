#include "Reader.hpp"

#include <string.h>

Reader::Reader()
{
	mBuffer = NULL;
	mLength = 0;
	mPos = 0;
	mBit = 0;
}

Reader::~Reader()
{
}

void Reader::setBuffer(const unsigned char *buffer, int length)
{
	mBuffer = buffer;
	mLength = length;

	updateCurrent();
}

unsigned int Reader::readBits(int num)
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

int Reader::readBytes(unsigned char *buffer, int length)
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

void Reader::byteSync()
{
	if(mBit > 0) {
		mPos++;
		mBit = 0;

		updateCurrent();
	}
}

bool Reader::empty()
{
	return (mLength == 0);
}

void Reader::updateCurrent()
{
	if(mPos >= mLength) {
		mPos = 0;
		mLength = refillBuffer();
	}

	if(mPos < mLength) {
		mCurrent = mBuffer[mPos];
	}
}

BufferReader::BufferReader(const unsigned char *buffer, int length)
{
	setBuffer(buffer, length);
}

int BufferReader::refillBuffer()
{
	return 0;
}

#define BUFFER_SIZE 4096
FileReader::FileReader(FILE *file) 
{
	mFile = file;
	mMutableBuffer = new unsigned char[BUFFER_SIZE];
	setBuffer(mMutableBuffer, 0);
}

FileReader::~FileReader()
{
	delete[] mMutableBuffer;
}

int FileReader::refillBuffer()
{
	return fread(mMutableBuffer, 1, BUFFER_SIZE, mFile);
}
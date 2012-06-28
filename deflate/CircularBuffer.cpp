#include "CircularBuffer.hpp"

CircularBuffer::CircularBuffer(int size)
{
	mBuffer = new unsigned char[size];
	mSize = size;
	mWritePos = 0;
}

CircularBuffer::~CircularBuffer()
{
	delete[] mBuffer;
}

void CircularBuffer::putByte(unsigned char byte)
{
	mBuffer[mWritePos] = byte;
	mWritePos++;
	if(mWritePos == mSize) {
		mWritePos = 0;
	}
}

void CircularBuffer::copy(int distance, int length)
{
	int src;
	int i;

	src = mWritePos + mSize - distance;
	if(src >= mSize) {
		src -= mSize;
	}

	for(i=0; i<length; i++) {
		mBuffer[mWritePos] = mBuffer[src];
		mWritePos++;
		if(mWritePos == mSize) {
			mWritePos = 0;
		}

		src++;
		if(src == mSize) {
			src = 0;
		}
	}
}

void CircularBuffer::write(const unsigned char *buffer, int length)
{
	int i;

	for(i=0; i<length; i++) {
		mBuffer[mWritePos] = buffer[i];
		mWritePos++;
		if(mWritePos == mSize) {
			mWritePos = 0;
		}
	}
}

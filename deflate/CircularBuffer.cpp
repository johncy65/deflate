#include <string.h>

#include "CircularBuffer.hpp"
#include "BitReader.hpp"

CircularBuffer::CircularBuffer(int size)
{
	mBuffer = new unsigned char[size];
	mSize = size;
	mWritePos = 0;
	mReadPos = 0;
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
	int writeLength = length;
	if(writeLength > writeRemaining()) {
		writeLength = writeRemaining();
	}

	int bytesWritten = 0;
	while(bytesWritten < writeLength)
	{
		int src = mWritePos - distance;
		if(src < 0) {
			src += mSize;
		}

		int segmentLength = writeLength;
		if(segmentLength > mSize - mWritePos) {
			segmentLength = mSize - mWritePos;
		}

		if(segmentLength > mSize - src) {
			segmentLength = mSize - src;
		}

		memcpy(mBuffer + mWritePos, mBuffer + src, segmentLength);
		bytesWritten += segmentLength;
		mWritePos += segmentLength;

		if(mWritePos == mSize) {
			mWritePos = 0;
		}
	}
}

int CircularBuffer::write(const unsigned char *buffer, int length)
{
	int writeLength = length;
	if(writeLength > writeRemaining()) {
		writeLength = writeRemaining();
	}

	int bytesWritten = 0;

	while(bytesWritten < writeLength) {
		int segmentLength = writeLength - bytesWritten;
		if(segmentLength > mSize - mWritePos) {
			segmentLength = mSize - mWritePos;
		}

		memcpy(mBuffer + mWritePos, buffer + bytesWritten, segmentLength);
		bytesWritten += segmentLength;
		mWritePos += segmentLength;

		if(mWritePos == mSize) {
			mWritePos = 0;
		}
	}

	return bytesWritten;
}

int CircularBuffer::write(BitReader *reader, int length)
{
	int writeLength = length;
	if(writeLength > writeRemaining()) {
		writeLength = writeRemaining();
	}

	int bytesWritten = 0;
	while(bytesWritten < writeLength) {
		if(reader->empty()) {
			break;
		}

		int segmentLength = writeLength - bytesWritten;
		if(segmentLength > mSize - mWritePos) {
			segmentLength = mSize - mWritePos;
		}

		int len = reader->readBytes(mBuffer + mWritePos, segmentLength);
		bytesWritten += len;
		mWritePos += len;

		if(mWritePos == mSize) {
			mWritePos = 0;
		}
	}

	return bytesWritten;
}

int CircularBuffer::readRemaining()
{
	int ret = mWritePos - mReadPos;
	if(ret < 0) {
		ret += mSize;
	}

	return ret;
}

int CircularBuffer::writeRemaining()
{
	return mSize - readRemaining();
}

int CircularBuffer::read(unsigned char *buffer, int length)
{
	int readLength = length;
	if(readLength > readRemaining()) {
		readLength = readRemaining();
	}

	int bytesRead = 0;
	while(bytesRead < readLength) {
		int segmentLength = readLength - bytesRead;
		if(segmentLength > mSize - mReadPos) {
			segmentLength = mSize - mReadPos;
		}

		memcpy(buffer + bytesRead, mBuffer + mReadPos, segmentLength);
		bytesRead += segmentLength;
		mReadPos += segmentLength;
		if(mReadPos == mSize) {
			mReadPos = 0;
		}
	}

	return bytesRead;
}
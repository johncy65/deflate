#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

class Reader;

class CircularBuffer {
public:
	CircularBuffer(int size);
	~CircularBuffer();

	void putByte(unsigned char byte);
	void copy(int distance, int length);
	int write(const unsigned char *buffer, int length);
	int write(Reader *reader, int length);

	int readRemaining();
	int writeRemaining();
	int read(unsigned char *buffer, int length);
private:
	unsigned char *mBuffer;
	int mSize;
	int mWritePos;
	int mReadPos;
};

#endif
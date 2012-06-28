#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

class CircularBuffer {
public:
	CircularBuffer(int size);
	~CircularBuffer();

	void putByte(unsigned char byte);
	void copy(int distance, int length);
	void write(const unsigned char *buffer, int length);

private:
	unsigned char *mBuffer;
	int mSize;
	int mWritePos;
};

#endif
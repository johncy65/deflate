#ifndef INFLATE_H
#define INFLATE_H

#include "BitReader.hpp"
#include "HuffmanTree.hpp"
#include "CircularBuffer.hpp"

class Inflate {
public:
	Inflate(const unsigned char *buffer, int buffer_size);
	~Inflate();

	int read(unsigned char *buffer, int length);
	bool empty();

private:
	void setupBlock();
	int readBlock(unsigned char *buffer, int length);

	BitReader *mReader;
	unsigned int mBlockFinal;
	unsigned int mBlockType;
	HuffmanTree *mLitlengthTree;
	HuffmanTree *mDistTree;
	int mRawLength;
	bool mBlockEmpty;
	CircularBuffer *mBuffer;
};

#endif

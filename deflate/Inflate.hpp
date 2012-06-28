#ifndef INFLATE_H
#define INFLATE_H

#include "BitReader.hpp"
#include "HuffmanTree.hpp"
#include "CircularBuffer.hpp"

class Inflate {
public:
	Inflate(const unsigned char *buffer, int buffer_size);
	~Inflate();

	void read();

private:
	void setupBlock();
	void readBlock();

	BitReader *mReader;
	unsigned int mBlockFinal;
	unsigned int mBlockType;
	HuffmanTree *mLitlengthTree;
	HuffmanTree *mDistTree;
	CircularBuffer *mBuffer;
};

#endif

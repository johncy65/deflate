#ifndef INFLATE_H
#define INFLATE_H

#include "Reader.hpp"
#include "HuffmanTree.hpp"
#include "CircularBuffer.hpp"

class Inflate {
public:
	Inflate(Reader *reader);
	~Inflate();

	int read(unsigned char *buffer, int length);
	bool empty();

private:
	void setupBlock();
	int readBlock(unsigned char *buffer, int length);

	Reader *mReader;
	unsigned int mBlockFinal;
	unsigned int mBlockType;
	HuffmanTree *mLitlengthTree;
	HuffmanTree *mDistTree;
	int mRawLength;
	bool mBlockEmpty;
	CircularBuffer *mBuffer;
};

#endif

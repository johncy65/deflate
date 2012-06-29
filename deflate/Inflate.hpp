#ifndef INFLATE_H
#define INFLATE_H

#include <exception>

#include "Reader.hpp"
#include "HuffmanTree.hpp"
#include "CircularBuffer.hpp"

class Inflate {
public:
	class ReadException : public std::exception
	{
	public:
		ReadException(int position) {
			mPosition = position;
		}

		int position() { return mPosition; }

	private:
		int mPosition;
	};

	Inflate(Reader &reader);
	~Inflate();

	int read(unsigned char *buffer, int length) throw(ReadException);

private:
	void setupBlock() throw(Reader::EndException);
	void readDynamicHuffmanTables() throw(ReadException);
	void throwException() throw(ReadException);

	Reader &mReader;
	bool mBlockFinal;
	int mBlockType;
	HuffmanTree *mLitlengthTree;
	HuffmanTree *mDistTree;
	bool mBlockEmpty;
	CircularBuffer mBuffer;
};

#endif

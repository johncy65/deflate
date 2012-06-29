#include <stdlib.h>
#include <string.h>

#include "Inflate.hpp"
#include "Reader.hpp"
#include "CircularBuffer.hpp"
#include "HuffmanTree.hpp"

#define BTYPE_NONE 0x0
#define BTYPE_FIXED 0x1
#define BTYPE_DYNAMIC 0x2
#define BTYPE_RESERVED 0x3

#define CLEN_COPY 16
#define CLEN_ZERO_3 17
#define CLEN_ZERO_7 18
#define CLEN_MAX 18

#define NUM_LENGTH_CODES 19
#define NUM_LITLENGTH_CODES 288
#define NUM_DIST_CODES 32

#define BUFFER_SIZE 32768

typedef struct {
	int bits;
	int val;
} BitVal;

const BitVal lengthBitvals[] = {
	{ 0, 3 },   { 0, 4 },   { 0, 5 },   { 0, 6 },  { 0, 7 },  { 0, 8 },  { 0, 9 },  { 0, 10 },
	{ 1, 11 },  { 1, 13 },  { 1, 15 },  { 1, 17 }, { 2, 19 }, { 2, 23 }, { 2, 27 }, { 2, 31 },
	{ 3, 35 },  { 3, 43 },  { 3, 51 },  { 3, 59 },  { 4, 67 }, { 4, 83 }, { 4, 99 }, { 4, 115 },
	{ 5, 131 },	{ 5, 163 }, { 5, 195 }, { 5, 227 }, { 0, 258 }
};

const BitVal distBitvals[] = {
	{ 0, 1 },      { 0, 2 },     { 0, 3 },      { 0, 4 },      { 1, 5 },      { 1, 7 },      { 2, 9 },
	{ 2, 13 },     { 3, 17 },    { 3, 25 },     { 4, 33 },     { 4, 49 },     { 5, 65 },     { 5, 97 },
	{ 6, 129 },    { 6, 193 },	 { 7, 257 },    { 7, 385 },    { 8, 513 },    { 8, 769 },    { 9, 1025 },
	{ 9, 1537 },   { 10, 2049 }, { 10, 3073 }, { 11, 4097 },  { 11, 6145 },  { 12, 8193 },  { 12, 12289 }, 
	{ 13, 16385 }, { 13, 24577 }
};

static const int lengthsOrder[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

Inflate::Inflate(Reader *reader)
{
	mReader = reader;
	mBuffer = new CircularBuffer(BUFFER_SIZE);
	mBlockEmpty = true;
	mBlockFinal = 0;
}

Inflate::~Inflate()
{
	delete mBuffer;
	delete mLitlengthTree;
	delete mDistTree;
}

void Inflate::setupBlock()
{
	int lengths[NUM_LITLENGTH_CODES + NUM_DIST_CODES];
	int i;

	mBlockFinal = mReader->readBits(1);
	mBlockType = mReader->readBits(2);
	if(mBlockType == BTYPE_RESERVED) {
		throw ReadException(mReader->position());
	}

	mBlockEmpty = false;

	switch(mBlockType) {
		case BTYPE_NONE:
			{
				mLitlengthTree = NULL;
				mDistTree = NULL;
				mReader->byteSync();
				mRawLength = mReader->readBits(16);
				unsigned int nlen = mReader->readBits(16);

				if(mRawLength != ~nlen) {
					throwException();
				}

				break;
			}

		case BTYPE_FIXED:
			mRawLength = 0;

			for(i=0; i<288; i++) {
				if(i < 144) {
					lengths[i] = 8;
				} else if(i < 256) {
					lengths[i] = 9;
				} else if(i < 280) {
					lengths[i] = 7;
				} else {
					lengths[i] = 8;
				}
			}

			mLitlengthTree = HuffmanTree::fromLengths(lengths, NUM_LITLENGTH_CODES);

			for(i=0; i<32; i++) {
				lengths[i] = 5;
			}

			mDistTree = HuffmanTree::fromLengths(lengths, NUM_DIST_CODES);

			break;

		case BTYPE_DYNAMIC:
			{
				mRawLength = 0;

				unsigned int hlit = mReader->readBits(5);
				unsigned int hdist = mReader->readBits(5);
				unsigned int hclen = mReader->readBits(4);

				if(hlit > 29 || hdist > 31 || hclen > 15) {
					throwException();
				}

				memset(lengths, 0, NUM_LENGTH_CODES * sizeof(int));
				for(i=0; i<hclen+4; i++) {
					lengths[lengthsOrder[i]] = mReader->readBits(3);
				}

				HuffmanTree *tree = HuffmanTree::fromLengths(lengths, NUM_LENGTH_CODES);

				memset(lengths, 0, (NUM_LITLENGTH_CODES + NUM_DIST_CODES) * sizeof(int));
				unsigned int lastCodeword = 0;
				i = 0;
				while(i < hlit + hdist + 258) {
					unsigned int codeword = tree->read(mReader);

					if(codeword > CLEN_MAX) {
						throwException();
					}

					switch(codeword) {
						case CLEN_COPY:
							{
							unsigned int extra = mReader->readBits(2);
							for(int j=0; j<extra + 3; j++) {
								lengths[i++] = lastCodeword;

								if(i >= hlit + hdist + 258) {
									throwException();
								}
							}
							break;
							}

						case CLEN_ZERO_3:
							{
							unsigned int extra = mReader->readBits(3);
							for(int j=0; j<extra + 3; j++) {
								lengths[i++] = 0;

								if(i >= hlit + hdist + 258) {
									throwException();
								}
							}
							lastCodeword = 0;
							break;
							}

						case CLEN_ZERO_7:
							{
							unsigned int extra = mReader->readBits(7);
							for(int j=0; j<extra + 11; j++) {
								lengths[i++] = 0;

								if(i >= hlit + hdist + 258) {
									throwException();
								}
							}
							lastCodeword = 0;
							break;
							}

						default:
							lengths[i++] = codeword;
							lastCodeword = codeword;
							break;
					}
				}

				delete tree;

				mLitlengthTree = HuffmanTree::fromLengths(lengths, hlit + 257);
				mDistTree = HuffmanTree::fromLengths(lengths + hlit + 257, hdist + 1);
				break;
			}
	}
}

int Inflate::readBlock(unsigned char *buffer, int length)
{
	int bytesRead = mBuffer->read(buffer, length);

	while(bytesRead < length) {
		if(mReader->empty()) {
			mBlockEmpty = true;
		}

		if(mBlockEmpty) {
			break;
		}

		switch(mBlockType) {
			case BTYPE_NONE:
				mRawLength -= mBuffer->write(mReader, mRawLength);
				if(mRawLength == 0) {
					mBlockEmpty = true;
				}
				break;

			case BTYPE_FIXED:
			case BTYPE_DYNAMIC:
				{
					unsigned int codeword = mLitlengthTree->read(mReader);

					if(codeword > 285) {
						throwException();
					}

					if(codeword < 256) {
						mBuffer->putByte((unsigned char)codeword);
					} else if(codeword == 256) {
						mBlockEmpty = true;
					} else {
						BitVal bitval = lengthBitvals[codeword - 257];
						unsigned int extra = mReader->readBits(bitval.bits);
						int len = bitval.val + extra;

						codeword = mDistTree->read(mReader);
						if(codeword > 29) {
							throwException();
						}

						bitval = distBitvals[codeword];
						extra = mReader->readBits(bitval.bits);
						int dist = bitval.val + extra;

						mBuffer->copy(dist, len);
					}

					break;
				}
		}

		bytesRead += mBuffer->read(buffer + bytesRead, length - bytesRead);
	}

	return bytesRead;
}

int Inflate::read(unsigned char *buffer, int length)
{
	try {
		int bytesRead = 0;
		while(bytesRead < length) {
			if(mBlockEmpty) {
				if(mBlockFinal) {
					break;
				}
				setupBlock();
			}

			bytesRead += readBlock(buffer + bytesRead, length - bytesRead);
		}

		return bytesRead;
	} catch(Reader::EndException e) {
		throw ReadException(e.position());
	}
}

bool Inflate::empty()
{
	return (mBlockEmpty && mBlockFinal);
}

void Inflate::throwException()
{
	throw ReadException(mReader->position());
}
#include <stdlib.h>
#include <string.h>

#include "Inflate.hpp"
#include "BitReader.hpp"
#include "CircularBuffer.hpp"
#include "HuffmanTree.hpp"

#define BTYPE_NONE 0x0
#define BTYPE_FIXED 0x1
#define BTYPE_DYNAMIC 0x2
#define BTYPE_RESERVED 0x3

#define CLEN_COPY 16
#define CLEN_ZERO_3 17
#define CLEN_ZERO_7 18

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

void Inflate::setupBlock()
{
	unsigned int hlit;
	unsigned int hdist;
	unsigned int hclen;
	int lengths[NUM_LITLENGTH_CODES + NUM_DIST_CODES];
	int i;
	int j;
	unsigned int extra;
	unsigned int codeword;
	unsigned int lastCodeword;
	HuffmanTree *tree;

	mBlockFinal = mReader->read(1);
	mBlockType = mReader->read(2);

	switch(mBlockType) {
		case BTYPE_NONE:
			mLitlengthTree = NULL;
			mDistTree = NULL;
			break;

		case BTYPE_FIXED:
			for(i=0; i<144; i++) {
				lengths[i] = 8;
			}

			for(i=144; i<256; i++) {
				lengths[i] = 9;
			}

			for(i=256; i<280; i++) {
				lengths[i] = 7;
			}

			for(i=280; i<288; i++) {
				lengths[i] = 8;
			}

			mLitlengthTree = HuffmanTree::fromLengths(lengths, NUM_LITLENGTH_CODES);

			for(i=0; i<32; i++) {
				lengths[i] = 5;
			}

			mDistTree = HuffmanTree::fromLengths(lengths, NUM_DIST_CODES);

			break;

		case BTYPE_DYNAMIC:
			hlit = mReader->read(5);
			hdist = mReader->read(5);
			hclen = mReader->read(4);

			memset(lengths, 0, NUM_LENGTH_CODES * sizeof(int));
			for(i=0; i<hclen+4; i++) {
				lengths[lengthsOrder[i]] = mReader->read(3);
			}

			tree = HuffmanTree::fromLengths(lengths, NUM_LENGTH_CODES);

			memset(lengths, 0, (NUM_LITLENGTH_CODES + NUM_DIST_CODES) * sizeof(int));
			lastCodeword = 0;
			i = 0;
			while(i < hlit + hdist + 258) {
				codeword = tree->read(mReader);

				switch(codeword) {
					case CLEN_COPY:
						extra = mReader->read(2);
						for(j=0; j<extra + 3; j++) {
							lengths[i++] = lastCodeword;
						}
						break;

					case CLEN_ZERO_3:
						extra = mReader->read(3);
						for(j=0; j<extra + 3; j++) {
							lengths[i++] = 0;
						}
						lastCodeword = 0;
						break;

					case CLEN_ZERO_7:
						extra = mReader->read(7);
						for(j=0; j<extra + 11; j++) {
							lengths[i++] = 0;
						}
						lastCodeword = 0;
						break;

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

void Inflate::readBlock()
{
	int codeword;
	int length;
	int distance;
	BitVal bitval;
	unsigned int extra;

	if(mBlockType == BTYPE_NONE) {
		mReader->byteSync();
		length = mReader->read(16);
		mReader->read(16);
		mBuffer->write(mReader->buffer(), length);
		mReader->advance(length);
		return;
	}

	while(1) {
		codeword = mLitlengthTree->read(mReader);

		if(codeword < 256) {
			mBuffer->putByte((unsigned char)codeword);
		} else if(codeword == 256) {
			break;
		} else {
			bitval = lengthBitvals[codeword - 257];
			extra = mReader->read(bitval.bits);
			length = bitval.val + extra;

			codeword = mDistTree->read(mReader);
			bitval = distBitvals[codeword];
			extra = mReader->read(bitval.bits);
			distance = bitval.val + extra;

			mBuffer->copy(distance, length);
		}
	}
}

void Inflate::read()
{
	while(1) {
		readBlock();

		if(mBlockFinal) {
			break;
		}

		setupBlock();
	}
}

Inflate::Inflate(const unsigned char *buffer, int bufferSize)
{
	mReader = new BitReader(buffer, bufferSize);
	mBuffer = new CircularBuffer(BUFFER_SIZE);

	setupBlock();
}

Inflate::~Inflate()
{
	delete mReader;
	delete mBuffer;
	delete mLitlengthTree;
	delete mDistTree;
}

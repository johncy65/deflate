#include "Inflate.hpp"
#include "Reader.hpp"
#include "CircularBuffer.hpp"
#include "HuffmanTree.hpp"

static const int BlockTypeNoCompression = 0;
static const int BlockTypeFixedTables = 1;
static const int BlockTypeDynamicTables = 2;
static const int BlockTypeReserved = 3;

static const int CodeLengthCopy = 16;
static const int CodeLengthZero3 = 17;
static const int CodeLengthZero7 = 18;
static const int CodeLengthMax = CodeLengthZero7;

static const int NumLengthCodes = 19;
static const int NumLitlengthCodes = 288;
static const int NumDistCodes = 32;

static const int CircularBufferSize = 65536;

typedef struct {
	int bits;
	int val;
} BitVal;

static const BitVal lengthBitvals[] = {
	{ 0, 3 },   { 0, 4 },   { 0, 5 },   { 0, 6 },  { 0, 7 },  { 0, 8 },  { 0, 9 },  { 0, 10 },
	{ 1, 11 },  { 1, 13 },  { 1, 15 },  { 1, 17 }, { 2, 19 }, { 2, 23 }, { 2, 27 }, { 2, 31 },
	{ 3, 35 },  { 3, 43 },  { 3, 51 },  { 3, 59 },  { 4, 67 }, { 4, 83 }, { 4, 99 }, { 4, 115 },
	{ 5, 131 },	{ 5, 163 }, { 5, 195 }, { 5, 227 }, { 0, 258 }
};

static const BitVal distBitvals[] = {
	{ 0, 1 },      { 0, 2 },     { 0, 3 },      { 0, 4 },      { 1, 5 },      { 1, 7 },      { 2, 9 },
	{ 2, 13 },     { 3, 17 },    { 3, 25 },     { 4, 33 },     { 4, 49 },     { 5, 65 },     { 5, 97 },
	{ 6, 129 },    { 6, 193 },	 { 7, 257 },    { 7, 385 },    { 8, 513 },    { 8, 769 },    { 9, 1025 },
	{ 9, 1537 },   { 10, 2049 }, { 10, 3073 }, { 11, 4097 },  { 11, 6145 },  { 12, 8193 },  { 12, 12289 }, 
	{ 13, 16385 }, { 13, 24577 }
};

static const int lengthsOrder[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

Inflate::Inflate(Reader &reader)
: mReader(reader)
, mBuffer(CircularBufferSize)
{
	mBlockEmpty = true;
	mBlockFinal = 0;
	mLitlengthTree = NULL;
	mDistTree = NULL;
}

Inflate::~Inflate()
{
	delete mLitlengthTree;
	delete mDistTree;
}

void Inflate::readDynamicHuffmanTables()
{
	unsigned int hlit = mReader.readBits(5);
	unsigned int hdist = mReader.readBits(5);
	unsigned int hclen = mReader.readBits(4);

	if(hlit > 29 || hdist > 31 || hclen > 15) {
		throwException();
	}

	int lengthLengths[NumLengthCodes];
	memset(lengthLengths, 0, NumLengthCodes * sizeof(int));
	for(int i=0; i<hclen+4; i++) {
		lengthLengths[lengthsOrder[i]] = mReader.readBits(3);
	}

	HuffmanTree *tree = HuffmanTree::fromLengths(lengthLengths, NumLengthCodes);

	int lengths[NumLitlengthCodes + NumDistCodes];
	memset(lengths, 0, (NumLitlengthCodes + NumDistCodes) * sizeof(int));
	unsigned int lastCodeword = 0;
	int i = 0;
	while(i < hlit + hdist + 258) {
		unsigned int codeword = tree->read(mReader);

		if(codeword > CodeLengthMax) {
			throwException();
		}

		switch(codeword) {
			case CodeLengthCopy:
				{
				unsigned int extra = mReader.readBits(2);
				for(int j=0; j<extra + 3; j++) {
					lengths[i++] = lastCodeword;

					if(i >= hlit + hdist + 258) {
						throwException();
					}
				}
				break;
				}

			case CodeLengthZero3:
				{
				unsigned int extra = mReader.readBits(3);
				for(int j=0; j<extra + 3; j++) {
					lengths[i++] = 0;

					if(i >= hlit + hdist + 258) {
						throwException();
					}
				}
				lastCodeword = 0;
				break;
				}

			case CodeLengthZero7:
				{
				unsigned int extra = mReader.readBits(7);
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
}

void Inflate::setupBlock()
{
	delete mLitlengthTree;
	mLitlengthTree = NULL;

	delete mDistTree;
	mDistTree = NULL;

	mBlockFinal = (mReader.readBits(1) == 1);
	mBlockType = mReader.readBits(2);
	if(mBlockType == BlockTypeReserved) {
		throw ReadException(mReader.position());
	}

	mBlockEmpty = false;

	switch(mBlockType) {
		case BlockTypeFixedTables:
			{
				int litlengthLengths[NumLitlengthCodes];
				for(int i=0; i<288; i++) {
					if(i < 144) {
						litlengthLengths[i] = 8;
					} else if(i < 256) {
						litlengthLengths[i] = 9;
					} else if(i < 280) {
						litlengthLengths[i] = 7;
					} else {
						litlengthLengths[i] = 8;
					}
				}

				int distLengths[NumDistCodes];
				for(int i=0; i<32; i++) {
					distLengths[i] = 5;
				}

				mLitlengthTree = HuffmanTree::fromLengths(litlengthLengths, NumLitlengthCodes);
				mDistTree = HuffmanTree::fromLengths(distLengths, NumDistCodes);
				break;
			}

		case BlockTypeDynamicTables:
			{
				readDynamicHuffmanTables();
				break;
			}
	}
}

int Inflate::read(unsigned char *buffer, int length)
{
	try {
		int bytesRead = mBuffer.read(buffer, length);

		while(bytesRead < length) {
			if(mBlockEmpty) {
				if(mBlockFinal) {
					break;
				}

				setupBlock();
			}

			switch(mBlockType) {
				case BlockTypeNoCompression:
					{
						mReader.byteSync();

						unsigned int len = mReader.readBits(16);
						unsigned int nlen = mReader.readBits(16);

						if(len != ~nlen) {
							throwException();
						}

						mBuffer.write(mReader, len);
						mBlockEmpty = true;
						break;
					}

				case BlockTypeFixedTables:
				case BlockTypeDynamicTables:
					{
						unsigned int codeword = mLitlengthTree->read(mReader);

						if(codeword > 285) {
							throwException();
						}

						if(codeword < 256) {
							mBuffer.putByte((unsigned char)codeword);
						} else if(codeword == 256) {
							mBlockEmpty = true;
						} else {
							BitVal bitval = lengthBitvals[codeword - 257];
							unsigned int extra = mReader.readBits(bitval.bits);
							int len = bitval.val + extra;

							codeword = mDistTree->read(mReader);
							if(codeword > 29) {
								throwException();
							}

							bitval = distBitvals[codeword];
							extra = mReader.readBits(bitval.bits);
							int dist = bitval.val + extra;

							mBuffer.copy(dist, len);
						}

						break;
					}
			}

			bytesRead += mBuffer.read(buffer + bytesRead, length - bytesRead);
		}

		return bytesRead;
	} catch(Reader::EndException e) {
		throw ReadException(e.position());
	}
}

void Inflate::throwException()
{
	throw ReadException(mReader.position());
}

#include "HuffmanTree.hpp"
#include "BitReader.hpp"

#include <stdlib.h>

#define INVALID_CODEWORD 0xffffffff

HuffmanTree::HuffmanTree(unsigned int codeword)
{
	mCodeword = codeword;
	mChildren[0] = mChildren[1] = NULL;
}

HuffmanTree::HuffmanTree(HuffmanTree *child0, HuffmanTree *child1)
{
	mCodeword = INVALID_CODEWORD;
	mChildren[0] = child0;
	mChildren[1] = child1;
}

HuffmanTree *HuffmanTree::fromLengths(int *lengths, int numLengths)
{
	HuffmanTree **currentTrees;
	HuffmanTree **newTrees;
	HuffmanTree **trees;
	HuffmanTree **swap;
	HuffmanTree *result;
	int maxLength;
	int i;
	int treeIdx;
	int length;

	maxLength = 0;
	for(i=0; i<numLengths; i++) {
		if(lengths[i] > maxLength) {
			maxLength = lengths[i];
		}
	}

	trees = new HuffmanTree*[numLengths * 2 + 1];
	currentTrees = trees;
	newTrees = trees + numLengths;
	currentTrees[0] = NULL;

	for(length = maxLength; length >= 0; length--) {
		treeIdx = 0;

		if(length > 0) {
			for(i=0; i<numLengths; i++) {
				if(lengths[i] == length) {
					HuffmanTree *tree = new HuffmanTree(i);
					newTrees[treeIdx] = tree;
					treeIdx++;
				}
			}
		}

		for(i=0; ; i++) {
			HuffmanTree *tree;

			if(currentTrees[i] == NULL) {
				break;
			}

			tree = new HuffmanTree(currentTrees[i], currentTrees[i+1]);
			i++;
			newTrees[treeIdx] = tree;
			treeIdx++;
			if(currentTrees[i] == NULL) {
				break;
			}
		}

		newTrees[treeIdx] = NULL;
		swap = newTrees;
		newTrees = currentTrees;
		currentTrees = swap;
	}

	result = currentTrees[0];
	delete[] trees;

	return result;
}

HuffmanTree::~HuffmanTree()
{
	delete mChildren[0];
	delete mChildren[1];
}

unsigned int HuffmanTree::read(BitReader *reader)
{
	HuffmanTree *cursor = this;

	while(cursor->mCodeword == INVALID_CODEWORD) {
		unsigned int bit = reader->readBits(1);
		cursor = cursor->mChildren[bit];
	}

	return cursor->mCodeword;
}

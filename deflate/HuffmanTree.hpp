#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include "Reader.hpp"

class HuffmanTree {
public:
	static HuffmanTree *fromLengths(int *lengths, int numLengths);

	~HuffmanTree();

	unsigned int read(Reader &reader) throw(Reader::EndException);

private:
	HuffmanTree(unsigned int codeword);
	HuffmanTree(HuffmanTree *child0, HuffmanTree *child1);

	unsigned int mCodeword;
	HuffmanTree *mChildren[2];
};

#endif

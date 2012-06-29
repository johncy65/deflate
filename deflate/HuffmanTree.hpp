#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

class Reader;

class HuffmanTree {
public:
	static HuffmanTree *fromLengths(int *lengths, int numLengths);

	~HuffmanTree();

	unsigned int read(Reader *reader);

private:
	HuffmanTree(unsigned int codeword);
	HuffmanTree(HuffmanTree *child0, HuffmanTree *child1);

	unsigned int mCodeword;
	HuffmanTree *mChildren[2];
};

#endif
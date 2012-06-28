#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include "bit_reader.h"

struct _HuffmanTree;
typedef struct _HuffmanTree HuffmanTree;

HuffmanTree *huffman_tree_from_lengths(int *lengths, int num_lengths);
void huffman_tree_free(HuffmanTree *huffman_tree);

unsigned int huffman_tree_read(HuffmanTree *huffman_tree, BitReader *bit_reader);

#endif
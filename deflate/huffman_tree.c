#include "huffman_tree.h"

#include <stdlib.h>

struct _HuffmanTree {
	unsigned int codeword;
	HuffmanTree *children[2];
};

HuffmanTree *huffman_tree_from_lengths(int *lengths, int num_lengths)
{
	HuffmanTree **current_trees;
	HuffmanTree **new_trees;
	HuffmanTree **trees;
	HuffmanTree **swap;
	HuffmanTree *result;
	int max_length;
	int i;
	int tree_idx;
	int length;

	max_length = 0;
	for(i=0; i<num_lengths; i++) {
		if(lengths[i] > max_length) {
			max_length = lengths[i];
		}
	}

	trees = (HuffmanTree**)malloc((num_lengths * 2 + 1) * sizeof(HuffmanTree*));
	current_trees = trees;
	new_trees = trees + num_lengths;
	current_trees[0] = NULL;

	for(length = max_length; length >= 0; length--) {
		tree_idx = 0;

		if(length > 0) {
			for(i=0; i<num_lengths; i++) {
				if(lengths[i] == length) {
					HuffmanTree *tree = (HuffmanTree*)malloc(sizeof(HuffmanTree));
					tree->codeword = i;
					tree->children[0] = tree->children[1] = NULL;
					new_trees[tree_idx] = tree;
					tree_idx++;
				}
			}
		}

		for(i=0; ; i++) {
			HuffmanTree *tree;

			if(current_trees[i] == NULL) {
				break;
			}

			tree = (HuffmanTree*)malloc(sizeof(HuffmanTree));
			tree->codeword = -1;
			tree->children[0] = current_trees[i];
			i++;
			tree->children[1] = current_trees[i];
			new_trees[tree_idx] = tree;
			tree_idx++;
			if(current_trees[i] == NULL) {
				break;
			}
		}

		new_trees[tree_idx] = NULL;
		swap = new_trees;
		new_trees = current_trees;
		current_trees = swap;
	}

	result = current_trees[0];
	free(trees);
	return result;
}

void huffman_tree_free(HuffmanTree *huffman_tree)
{
	if(huffman_tree == NULL) {
		return;
	}

	huffman_tree_free(huffman_tree->children[0]);
	huffman_tree_free(huffman_tree->children[1]);
	free(huffman_tree);
}

unsigned int huffman_tree_read(HuffmanTree *huffman_tree, BitReader *bit_reader)
{
	HuffmanTree *cursor = huffman_tree;

	while(cursor->codeword == -1) {
		unsigned int bit = bit_reader_read(bit_reader, 1);
		cursor = cursor->children[bit];
	}

	return cursor->codeword;
}

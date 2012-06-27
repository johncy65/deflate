#include <stdlib.h>

#include "inflate.h"

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

#define OUT_BUFFER_SIZE 32768

typedef struct {
	int bits;
	int val;
} BitVal;

const BitVal length_bitvals[] = {
	{ 0, 3 },   { 0, 4 },   { 0, 5 },   { 0, 6 },  { 0, 7 },  { 0, 8 },  { 0, 9 },  { 0, 10 },
	{ 1, 11 },  { 1, 13 },  { 1, 15 },  { 1, 17 }, { 2, 19 }, { 2, 23 }, { 2, 27 }, { 2, 31 },
	{ 3, 35 },  { 3, 43 },  { 3, 51 },  { 3, 59 },  { 4, 67 }, { 4, 83 }, { 4, 99 }, { 4, 115 },
	{ 5, 131 },	{ 5, 163 }, { 5, 195 }, { 5, 227 }, { 0, 258 }
};

const BitVal dist_bitvals[] = {
	{ 0, 1 },      { 0, 2 },     { 0, 3 },      { 0, 4 },      { 1, 5 },      { 1, 7 },      { 2, 9 },
	{ 2, 13 },     { 3, 17 },    { 3, 25 },     { 4, 33 },     { 4, 49 },     { 5, 65 },     { 5, 97 },
	{ 6, 129 },    { 6, 193 },	 { 7, 257 },    { 7, 385 },    { 8, 513 },    { 8, 769 },    { 9, 1025 },
	{ 9, 1537 },   { 10, 2049 }, { 10, 3073 }, { 11, 4097 },  { 11, 6145 },  { 12, 8193 },  { 12, 12289 }, 
	{ 13, 16385 }, { 13, 24577 }
};

static const int lengths_order[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

struct _HuffmanTree;
typedef struct _HuffmanTree HuffmanTree;
struct _HuffmanTree {
	int codeword;
	HuffmanTree *children[2];
};

struct _Inflate {
	const unsigned char *in_buffer;
	int in_size;
	int in_pos;
	unsigned char current;
	int current_bit;
	unsigned int block_final;
	unsigned int block_type;
	HuffmanTree *litlength_tree;
	HuffmanTree *dist_tree;
	unsigned char out_buffer[OUT_BUFFER_SIZE];
	int out_write_pos;
};

static unsigned int getbits(Inflate *inflate, int n)
{
	unsigned int result = 0;
	int read = 0;
	while(read < n) {
		int left = n - read;
		unsigned int newbits = inflate->current;
		int x = 8 - inflate->current_bit;
		if(x > left) {
			x = left;
		}

		newbits &= ~(0xffffffff << x);
		result |= newbits << read;

		read += x;
		inflate->current >>= x;
		inflate->current_bit += x;
		if(inflate->current_bit == 8) {
			inflate->in_pos++;
			inflate->current = inflate->in_buffer[inflate->in_pos];
			inflate->current_bit = 0;
		}
	}

	return result;
}

static void byte_sync(Inflate *inflate)
{
	inflate->in_pos++;
	inflate->current = inflate->in_buffer[inflate->in_pos];
	inflate->current_bit = 0;
}

static HuffmanTree *build_tree(int *lengths, int num_lengths)
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

static void free_tree(HuffmanTree *tree)
{
	if(tree == NULL) {
		return;
	}

	free_tree(tree->children[0]);
	free_tree(tree->children[1]);
	free(tree);
}

static int read_huffman(Inflate *inflate, HuffmanTree *tree)
{
	HuffmanTree *cursor = tree;

	while(cursor->codeword == -1) {
		unsigned int bit = getbits(inflate, 1);
		cursor = cursor->children[bit];
	}

	return cursor->codeword;
}

static void read_lengths(Inflate *inflate, HuffmanTree *tree, int *lengths, int num_lengths, int *last_codeword)
{
	unsigned int extra;
	int codeword;
	int idx;
	int i;
	int j;

	idx = 0;
	while(idx < num_lengths) {
		codeword = read_huffman(inflate, tree);

		switch(codeword) {
			case CLEN_COPY:
				extra = getbits(inflate, 2);
				for(j=0; j<extra + 3; j++) {
					lengths[idx++] = *last_codeword;
				}
				break;

			case CLEN_ZERO_3:
				extra = getbits(inflate, 3);
				for(j=0; j<extra + 3; j++) {
					lengths[idx++] = 0;
				}
				*last_codeword = 0;
				break;

			case CLEN_ZERO_7:
				extra = getbits(inflate, 7);
				for(j=0; j<extra + 11; j++) {
					lengths[idx++] = 0;
				}
				*last_codeword = 0;
				break;

			default:
				lengths[idx++] = codeword;
				*last_codeword = codeword;
				break;
		}
	}
}

static void setup_block(Inflate *inflate)
{
	unsigned int hlit;
	unsigned int hdist;
	unsigned int hclen;
	int lengths[NUM_LENGTH_CODES];
	int lit_lengths[NUM_LITLENGTH_CODES];
	int dist_lengths[NUM_DIST_CODES];
	int i;
	int last_codeword;
	HuffmanTree *length_tree;

	inflate->block_final = getbits(inflate, 1);
	inflate->block_type = getbits(inflate, 2);

	switch(inflate->block_type) {
		case BTYPE_NONE:
			inflate->litlength_tree = NULL;
			inflate->dist_tree = NULL;
			break;

		case BTYPE_FIXED:
			for(i=0; i<144; i++) {
				lit_lengths[i] = 8;
			}

			for(i=144; i<256; i++) {
				lit_lengths[i] = 9;
			}

			for(i=256; i<280; i++) {
				lit_lengths[i] = 7;
			}

			for(i=280; i<288; i++) {
				lit_lengths[i] = 8;
			}

			for(i=0; i<32; i++) {
				dist_lengths[i] = 5;
			}

			inflate->litlength_tree = build_tree(lit_lengths, NUM_LITLENGTH_CODES);
			inflate->dist_tree = build_tree(dist_lengths, NUM_DIST_CODES);

			break;

		case BTYPE_DYNAMIC:
			hlit = getbits(inflate, 5);
			hdist = getbits(inflate, 5);
			hclen = getbits(inflate, 4);

			memset(lengths, 0, NUM_LENGTH_CODES * sizeof(int));
			for(i=0; i<hclen+4; i++) {
				lengths[lengths_order[i]] = getbits(inflate, 3);
			}

			length_tree = build_tree(lengths, NUM_LENGTH_CODES);

			memset(lit_lengths, 0, NUM_LITLENGTH_CODES * sizeof(int));
			memset(dist_lengths, 0, NUM_DIST_CODES * sizeof(int));
			last_codeword = 0;
			read_lengths(inflate, length_tree, lit_lengths, hlit + 257, &last_codeword);
			read_lengths(inflate, length_tree, dist_lengths, hdist + 1, &last_codeword);

			inflate->litlength_tree = build_tree(lit_lengths, NUM_LITLENGTH_CODES);
			inflate->dist_tree = build_tree(dist_lengths, NUM_DIST_CODES);

			free_tree(length_tree);
			break;
	}
}

static void put_out(Inflate *inflate, unsigned char c)
{
	inflate->out_buffer[inflate->out_write_pos] = c;
	inflate->out_write_pos++;
	if(inflate->out_write_pos == OUT_BUFFER_SIZE) {
		inflate->out_write_pos = 0;
	}
}

static void copy_out(Inflate *inflate, int length, int dist)
{
	int src;
	int i;

	src = inflate->out_write_pos + OUT_BUFFER_SIZE - dist;
	if(src >= OUT_BUFFER_SIZE) {
		src -= OUT_BUFFER_SIZE;
	}

	for(i=0; i<length; i++) {
		inflate->out_buffer[inflate->out_write_pos] = inflate->out_buffer[src];
		inflate->out_write_pos++;
		if(inflate->out_write_pos == OUT_BUFFER_SIZE) {
			inflate->out_write_pos = 0;
		}

		src++;
		if(src == OUT_BUFFER_SIZE) {
			src = 0;
		}
	}
}

static void write_out(Inflate *inflate, const unsigned char *buffer, int length)
{
	int i;

	for(i=0; i<length; i++) {
		inflate->out_buffer[inflate->out_write_pos] = buffer[i];
		inflate->out_write_pos++;
		if(inflate->out_write_pos == OUT_BUFFER_SIZE) {
			inflate->out_write_pos = 0;
		}
	}
}

static void read_block(Inflate *inflate)
{
	int codeword;
	int length;
	int dist;
	BitVal bitval;
	unsigned int extra;

	if(inflate->block_type == BTYPE_NONE) {
		byte_sync(inflate);
		length = getbits(inflate, 16);
		getbits(inflate, 16);
		write_out(inflate, inflate->in_buffer + inflate->in_pos, length);
		return;
	}

	while(1) {
		codeword = read_huffman(inflate, inflate->litlength_tree);

		if(codeword < 256) {
			put_out(inflate, (unsigned char)codeword);
		} else if(codeword == 256) {
			break;
		} else {
			bitval = length_bitvals[codeword - 257];
			extra = getbits(inflate, bitval.bits);
			length = bitval.val + extra;

			codeword = read_huffman(inflate, inflate->dist_tree);
			bitval = dist_bitvals[codeword];
			extra = getbits(inflate, bitval.bits);
			dist = bitval.val + extra;

			copy_out(inflate, length, dist);
		}
	}
}

void inflate_read(Inflate *inflate)
{
	while(1) {
		read_block(inflate);

		if(inflate->block_final) {
			break;
		}

		setup_block(inflate);
	}
}

Inflate *inflate_new(const unsigned char *buffer, int buffer_size)
{
	Inflate *inflate = (Inflate*)malloc(sizeof(Inflate));
	inflate->in_buffer = buffer;
	inflate->in_size = buffer_size;
	inflate->in_pos = 0;
	inflate->current = inflate->in_buffer[inflate->in_pos];
	inflate->current_bit = 0;
	inflate->out_write_pos = 0;

	setup_block(inflate);

	return inflate;
}

void inflate_free(Inflate *inflate)
{
	free_tree(inflate->litlength_tree);
	free_tree(inflate->dist_tree);
	free(inflate);
}

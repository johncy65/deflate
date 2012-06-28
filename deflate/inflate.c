#include <stdlib.h>

#include "inflate.h"
#include "bit_reader.h"
#include "circular_buffer.h"
#include "huffman_tree.h"

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

struct _Inflate {
	BitReader *reader;
	unsigned int block_final;
	unsigned int block_type;
	HuffmanTree *litlength_tree;
	HuffmanTree *dist_tree;
	CircularBuffer *buffer;
};

static void setup_block(Inflate *inflate)
{
	unsigned int hlit;
	unsigned int hdist;
	unsigned int hclen;
	int lengths[NUM_LITLENGTH_CODES + NUM_DIST_CODES];
	int i;
	int j;
	unsigned int extra;
	unsigned int codeword;
	unsigned int last_codeword;
	HuffmanTree *tree;

	inflate->block_final = bit_reader_read(inflate->reader, 1);
	inflate->block_type = bit_reader_read(inflate->reader, 2);

	switch(inflate->block_type) {
		case BTYPE_NONE:
			inflate->litlength_tree = NULL;
			inflate->dist_tree = NULL;
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

			inflate->litlength_tree = huffman_tree_from_lengths(lengths, NUM_LITLENGTH_CODES);

			for(i=0; i<32; i++) {
				lengths[i] = 5;
			}

			inflate->dist_tree = huffman_tree_from_lengths(lengths, NUM_DIST_CODES);

			break;

		case BTYPE_DYNAMIC:
			hlit = bit_reader_read(inflate->reader, 5);
			hdist = bit_reader_read(inflate->reader, 5);
			hclen = bit_reader_read(inflate->reader, 4);

			memset(lengths, 0, NUM_LENGTH_CODES * sizeof(int));
			for(i=0; i<hclen+4; i++) {
				lengths[lengths_order[i]] = bit_reader_read(inflate->reader, 3);
			}

			tree = huffman_tree_from_lengths(lengths, NUM_LENGTH_CODES);

			memset(lengths, 0, (NUM_LITLENGTH_CODES + NUM_DIST_CODES) * sizeof(int));
			last_codeword = 0;
			i = 0;
			while(i < hlit + hdist + 258) {
				codeword = huffman_tree_read(tree, inflate->reader);

				switch(codeword) {
					case CLEN_COPY:
						extra = bit_reader_read(inflate->reader, 2);
						for(j=0; j<extra + 3; j++) {
							lengths[i++] = last_codeword;
						}
						break;

					case CLEN_ZERO_3:
						extra = bit_reader_read(inflate->reader, 3);
						for(j=0; j<extra + 3; j++) {
							lengths[i++] = 0;
						}
						last_codeword = 0;
						break;

					case CLEN_ZERO_7:
						extra = bit_reader_read(inflate->reader, 7);
						for(j=0; j<extra + 11; j++) {
							lengths[i++] = 0;
						}
						last_codeword = 0;
						break;

					default:
						lengths[i++] = codeword;
						last_codeword = codeword;
						break;
				}
			}

			huffman_tree_free(tree);

			inflate->litlength_tree = huffman_tree_from_lengths(lengths, hlit + 257);
			inflate->dist_tree = huffman_tree_from_lengths(lengths + hlit + 257, hdist + 1);
			break;
	}
}

static void read_block(Inflate *inflate)
{
	int codeword;
	int length;
	int distance;
	BitVal bitval;
	unsigned int extra;

	if(inflate->block_type == BTYPE_NONE) {
		bit_reader_byte_sync(inflate->reader);
		length = bit_reader_read(inflate->reader, 16);
		bit_reader_read(inflate->reader, 16);
		circular_buffer_write(inflate->buffer, bit_reader_buffer(inflate->reader), length);
		bit_reader_advance(inflate->reader, length);
		return;
	}

	while(1) {
		codeword = huffman_tree_read(inflate->litlength_tree, inflate->reader);

		if(codeword < 256) {
			circular_buffer_put_byte(inflate->buffer, (unsigned char)codeword);
		} else if(codeword == 256) {
			break;
		} else {
			bitval = length_bitvals[codeword - 257];
			extra = bit_reader_read(inflate->reader, bitval.bits);
			length = bitval.val + extra;

			codeword = huffman_tree_read(inflate->dist_tree, inflate->reader);
			bitval = dist_bitvals[codeword];
			extra = bit_reader_read(inflate->reader, bitval.bits);
			distance = bitval.val + extra;

			circular_buffer_copy(inflate->buffer, distance, length);
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
	inflate->reader = bit_reader_new(buffer, buffer_size);
	inflate->buffer = circular_buffer_new(BUFFER_SIZE);

	setup_block(inflate);

	return inflate;
}

void inflate_free(Inflate *inflate)
{
	bit_reader_free(inflate->reader);
	circular_buffer_free(inflate->buffer);
	huffman_tree_free(inflate->litlength_tree);
	huffman_tree_free(inflate->dist_tree);

	free(inflate);
}

#include "bit_reader.h"

struct _BitReader {
	const unsigned char *buffer;
	int length;
	int pos;
	unsigned char current;
	int bit;
};

BitReader *bit_reader_new(const unsigned char *buffer, int length)
{
	BitReader *bit_reader = (BitReader*)malloc(sizeof(BitReader));

	bit_reader->buffer = buffer;
	bit_reader->length = length;
	bit_reader->pos = 0;
	bit_reader->current = bit_reader->buffer[bit_reader->pos];
	bit_reader->bit = 0;
}

void bit_reader_free(BitReader *bit_reader)
{
	free(bit_reader);
}

unsigned int bit_reader_read(BitReader *bit_reader, int num_bits)
{
	unsigned int result = 0;
	int read = 0;
	while(read < num_bits) {
		int left = num_bits - read;
		unsigned int newbits = bit_reader->current;
		int x = 8 - bit_reader->bit;
		if(x > left) {
			x = left;
		}

		newbits &= ~(0xffffffff << x);
		result |= newbits << read;

		read += x;
		bit_reader->current >>= x;
		bit_reader->bit += x;
		if(bit_reader->bit == 8) {
			bit_reader->pos++;
			bit_reader->current = bit_reader->buffer[bit_reader->pos];
			bit_reader->bit = 0;
		}
	}

	return result;
}

void bit_reader_byte_sync(BitReader *bit_reader)
{
	bit_reader->pos++;
	bit_reader->current = bit_reader->buffer[bit_reader->pos];
	bit_reader->bit = 0;
}

unsigned char *bit_reader_buffer(BitReader *bit_reader)
{
	return &bit_reader->buffer[bit_reader->pos];
}

void bit_reader_advance(BitReader *bit_reader, int length)
{
	bit_reader->pos += length;
	bit_reader->current = bit_reader->buffer[bit_reader->pos];
}

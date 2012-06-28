#ifndef BIT_READER_H
#define BIT_READER_H

struct _BitReader;
typedef struct _BitReader BitReader;

BitReader *bit_reader_new(const unsigned char *buffer, int length);
void bit_reader_free(BitReader *bit_reader);

unsigned int bit_reader_read(BitReader *bit_reader, int num_bits);
void bit_reader_byte_sync(BitReader *bit_reader);
unsigned char *bit_reader_buffer(BitReader *bit_reader);
void bit_reader_advance(BitReader *bit_reader, int length);

#endif
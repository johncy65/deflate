#include "circular_buffer.h"

struct _CircularBuffer {
	unsigned char *buffer;
	int size;
	int write_pos;
};

CircularBuffer *circular_buffer_new(int size)
{
	CircularBuffer *circular_buffer = (CircularBuffer*)malloc(sizeof(CircularBuffer));
	circular_buffer->buffer = (unsigned char*)malloc(size);
	circular_buffer->size = size;
	circular_buffer->write_pos = 0;

	return circular_buffer;
}

void circular_buffer_free(CircularBuffer *circular_buffer)
{
	free(circular_buffer->buffer);
	free(circular_buffer);
}

void circular_buffer_put_byte(CircularBuffer *circular_buffer, unsigned char byte)
{
	circular_buffer->buffer[circular_buffer->write_pos] = byte;
	circular_buffer->write_pos++;
	if(circular_buffer->write_pos == circular_buffer->size) {
		circular_buffer->write_pos = 0;
	}
}

void circular_buffer_copy(CircularBuffer *circular_buffer, int distance, int length)
{
	int src;
	int i;

	src = circular_buffer->write_pos + circular_buffer->size - distance;
	if(src >= circular_buffer->size) {
		src -= circular_buffer->size;
	}

	for(i=0; i<length; i++) {
		circular_buffer->buffer[circular_buffer->write_pos] = circular_buffer->buffer[src];
		circular_buffer->write_pos++;
		if(circular_buffer->write_pos == circular_buffer->size) {
			circular_buffer->write_pos = 0;
		}

		src++;
		if(src == circular_buffer->size) {
			src = 0;
		}
	}
}

void circular_buffer_write(CircularBuffer *circular_buffer, const unsigned char *buffer, int length)
{
	int i;

	for(i=0; i<length; i++) {
		circular_buffer->buffer[circular_buffer->write_pos] = buffer[i];
		circular_buffer->write_pos++;
		if(circular_buffer->write_pos == circular_buffer->size) {
			circular_buffer->write_pos = 0;
		}
	}
}

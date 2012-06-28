#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

struct _CircularBuffer;
typedef struct _CircularBuffer CircularBuffer;

CircularBuffer *circular_buffer_new(int size);
void circular_buffer_free(CircularBuffer *circular_buffer);

void circular_buffer_put_byte(CircularBuffer *circular_buffer, unsigned char byte);
void circular_buffer_copy(CircularBuffer *circular_buffer, int distance, int length);
void circular_buffer_write(CircularBuffer *circular_buffer, const unsigned char *buffer, int length);

#endif
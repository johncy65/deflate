#ifndef INFLATE_H
#define INFLATE_H

struct _Inflate;
typedef struct _Inflate Inflate;

Inflate *inflate_new(const unsigned char *buffer, int buffer_size);
void inflate_free(Inflate *inflate);

void inflate_read(Inflate *inflate);

#endif

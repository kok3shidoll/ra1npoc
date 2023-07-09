#ifndef LZ4_MAIN_H
#define LZ4_MAIN_H

#include <lz4.h>
#include <lz4hc.h>

int lz4CompressAndAddShellcode(const void *inbuf, const size_t insize, void **outbuf, size_t *outsz);

#endif

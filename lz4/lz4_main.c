#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/log.h>
#include <common/common.h>
#include <lz4.h>
#include <lz4hc.h>

#include <lz4dec_asm.h>

RA1NPOC_API int lz4CompressAndAddShellcode(const void *inbuf, const size_t insize, void **outbuf, size_t *outsize)
{
    if(insize > LZ4_MAX_INPUT_SIZE)
    {
        ERR("Input too large");
        *outbuf = NULL;
        return -1;
    }
    
    size_t tmpsize = LZ4_COMPRESSBOUND(insize);
    void *tmpbuf = malloc(tmpsize);
    if(!tmpbuf)
    {
        ERR("malloc: %s", strerror(errno));
        return -1;
    }
    
    int outlen = LZ4_compress_HC(inbuf, tmpbuf, (int)insize, (int)tmpsize, LZ4HC_CLEVEL_MAX);
    if(!outlen)
    {
        ERR("lz4 error");
        free(tmpbuf);
        *outbuf = NULL;
        return -1;
    }
    
    DEVLOG("Compressed 0x%zx bytes to 0x%llx bytes", insize, (unsigned long long)outlen);
    
    if(outlen > (0x40000 - 0x200))
    {
        ERR("pongoOS too large");
        free(tmpbuf);
        *outbuf = NULL;
        return -1;
    }
    
    *outbuf = malloc(outlen + 0x200);
    if(!*outbuf)
    {
        ERR("malloc: %s", strerror(errno));
        free(tmpbuf);
        *outbuf = NULL;
        return -1;
    }
    
    uint32_t* sizebuf = (uint32_t*)(lz4dec_bin + (0x200 - 4));
    sizebuf[0] = outlen;
    
    memcpy(*outbuf, lz4dec_bin, 0x200);
    memcpy(*outbuf+0x200, tmpbuf, outlen);
    free(tmpbuf);
    
    *outsize = outlen + 0x200;
    return 0;
}

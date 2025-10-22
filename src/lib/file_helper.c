#include "lib/file_helper.h"

#include <iconv.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void remove_r(char* code)
{
    if (*code == '\0')
        return;

    if (*code != '\r')
        remove_r(code + 1);
    else
    {
        memmove(code, code + 1, strlen(code + 1) + 1);
        remove_r(code);
    }
}

bool file_read(const char* path, const char* encoding, char* dst)
{
    FILE* fp = fopen(path, "rb");

    if (!fp)
        return false;

    fseek(fp, 0, SEEK_END);
    uint64_t size = ftell(fp);

    if (size <= 0)
    {
        fclose(fp);
        return false;
    }

    rewind(fp);

    char* src = (char*)(malloc((size + 16) * sizeof(char)));
    
    if (!src)
    {
        fclose(fp);
        return false;
    }

    uint64_t read_bytes = fread(src, 1, size, fp);
    src[read_bytes] = '\0';
    fclose(fp);

    iconv_t cd = iconv_open("UTF-8", encoding);
    
    if (cd == (iconv_t)(-1))
    {
        free(src);
        return false;
    }

    uint64_t inbytesleft = read_bytes;
    uint64_t outbytesleft = size * 4 + 4;
    char* inbuf = src;
    char* outbuf = dst;

    uint64_t result = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    if (result == (uint64_t)(-1))
    {
        iconv_close(cd);
        free(src);
        return false;
    }

    *outbuf = '\0';

    iconv_close(cd);
    free(src);
    remove_r(dst);
    return true;
}

bool file_write(const char* path, const char* content, bool append)
{
    FILE* fp = fopen(path, append ? "ab" : "wb");

    if (!fp)
        return false;

    const uint64_t len = strlen(content);
    fwrite(content, 1, len, fp);
    fclose(fp);
    return true;
}

bool file_delete(const char* path)
{
    return !remove(path);
}
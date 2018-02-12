#ifndef _TIT_SOX_BUF_H_
#define _TIT_SOX_BUF_H_
#include <string>

#include "Tit_Map.h"
typedef struct TIT_FILE
{
    char     *buf;
    size_t    buf_len;
    size_t    offset;
}TIT_FILE;

int    InBufSeek(std::string filename, long long offset, int whence);
int    OutBufSeek(std::string filename, long long offset, int whence);
bool   StoreTitBuff(std::string filename, char *buf, size_t len);
size_t GetSoxBufLen(std::string filename);
size_t CopySoxBuf(std::string filename, char *buff, size_t len);

#endif//_TIT_SOX_BUF_H_
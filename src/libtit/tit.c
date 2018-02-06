#include <stdlib.h>
extern size_t RaadTitBuff(const char * filename, void *buf, size_t len);
extern size_t WriteTitBuff(const char * filename, void const * buf, size_t len);

size_t Read_Tit_Buff(const char * filename, void *buf, size_t len)
{
  //printf("Read:  %s, len %lu; %s %s\n", filename, len, __DATE__, __TIME__);
  return ReadTitBuff(filename, buf, len);
}

size_t Write_Tit_Buff(const char * filename, void const * buf, size_t len)
{
  //printf("Write: %s, len %lu; %s %s\n", filename, len, __DATE__, __TIME__);
  return WriteTitBuff(filename, buf, len);
}


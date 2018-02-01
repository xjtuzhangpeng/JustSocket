#include <stdlib.h>
extern void * ReadTitBuff(const char * filename, void *buf, size_t len);
extern void * GetTitBuff(const char * filename, size_t buff_len, size_t sample_len);

size_t Read_Tit_Buff(const char * filename, void *buf, size_t len)
{
  //printf("Read:  %s, len %lu; %s %s\n", filename, len, __DATE__, __TIME__);
  return ReadTitBuff(filename, buf, len);
}

void * Get_Tit_Buff(const char * filename, size_t buff_len, size_t sample_len)
{
  //printf("%s, buff_len: %lu, sample_len: %lu; %s %s\n",
  //       filename, buff_len, sample_len, __DATE__, __TIME__);
  return GetTitBuff(filename, buff_len, sample_len);
}
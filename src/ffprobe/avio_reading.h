#ifndef _AVIO_READING_H_
#define _AVIO_READING_H_
#include "Tit_Logger.h"

#ifdef __cplusplus
extern "C" {
#endif
void InitSessionNum(int sessionNum);
int  GetInfo(int sessionId);
int  avio_reading_main(char *filename, int sessionid);

void tit_store_log(char * line, int sessionId);
#ifdef __cplusplus
}
#endif

#endif//_AVIO_READING_H_

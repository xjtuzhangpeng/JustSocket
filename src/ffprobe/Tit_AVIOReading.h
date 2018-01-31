#ifndef _AVIO_READING_H_
#define _AVIO_READING_H_

#ifdef __cplusplus
extern "C" {
#endif
void   InitSessionNum(int sessionNum);
size_t GetFormatInfo(int sessionId, char *buf, size_t buf_len);
size_t GetFormatInfoLen(int sessionId);
int    AVIOReading(char *filename, int sessionid);

void tit_store_log(char *line, int indx);
#ifdef __cplusplus
}
#endif

#endif//_AVIO_READING_H_

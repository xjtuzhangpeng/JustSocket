#ifndef _TIT_MONO_STEREO_H_
#define _TIT_MONO_STEREO_H_
#include "common.h"
#include "Tit_Resample.h"

#define _16BITS_SHORT_SIZE     sizeof(short)
#define OFFSET_BYTES_OF_DATA \
    (((sizeof(PCM_HEAD) + (_16BITS_SHORT_SIZE - 1)) / _16BITS_SHORT_SIZE) * _16BITS_SHORT_SIZE)

class HanldeFfmpegResult
{
public:
    HanldeFfmpegResult(char * buf, size_t len);
    ~HanldeFfmpegResult();

    size_t SwapBuff(char *&buf);
private:
    size_t GetResampleResult(char *in_ptr, size_t in_len, char ** out_ptr_ptr);
    void   SplitChannel();

    char         *m_buf;
    size_t        m_len;
    char         *m_buf_1;   // channel 1 
    size_t        m_len_1;   // channel 1 
    char         *m_buf_2;   // channel 2 
    size_t        m_len_2;   // channel 2 
    ChannelNum    m_chanlNum;
};

#endif//_TIT_MONO_STEREO_H_


#include "Tit_MonoStereo.h"

static const size_t gs_offset = OFFSET_BYTES_OF_DATA;

HanldeFfmpegResult::HanldeFfmpegResult(char * buf, size_t len)
    : m_buf(buf), m_len(len), 
      m_buf_1(NULL), m_len_1(0), 
      m_buf_2(NULL), m_len_2(0)
{
    PPCM_HEAD head = reinterpret_cast<PPCM_HEAD> (m_buf);
    m_chanlNum = static_cast<ChannelNum> (head->NumChannels);
    SplitChannel();
}

HanldeFfmpegResult::~HanldeFfmpegResult()
{
}

size_t HanldeFfmpegResult::GetResampleResult(char *in_ptr, size_t in_len, char ** out_ptr_ptr)
{
    PPCM_HEAD head = reinterpret_cast<PPCM_HEAD> (in_ptr);
    // 开始降或升采样 
    size_t inSize = in_len - PCM_HEAD_LEN;
    size_t oSize  = (size_t)(inSize * TBNR_INPUT_SAMPLES / head->SampleRate + .5);
    char *tmpBuf  = new char[oSize + PCM_HEAD_LEN];

    char *inBuf   = in_ptr + PCM_HEAD_LEN;
    char *oBuf    = tmpBuf + PCM_HEAD_LEN;

    TtsResample resample(head->SampleRate, TBNR_INPUT_SAMPLES);
    resample.ResampleBuf(reinterpret_cast<short *> (inBuf), CHAR_LEN_SHORT_LEN(inSize),
                         reinterpret_cast<short *> (oBuf),  CHAR_LEN_SHORT_LEN(oSize));

    // 重新赋值语音头
    head->ChunkID     = 0x46464952;
    head->NumChannels = 1;
    head->ByteRate    = (head->NumChannels * TBNR_INPUT_SAMPLES * head->BitsPerSample) >> 3;
    head->BlockAlign  = (head->NumChannels * head->BitsPerSample) >> 3;
    head->SampleRate  = TBNR_INPUT_SAMPLES;
    head->ChunkSize   = oSize + PCM_HEAD_LEN;
    head->DataTag     = 0x61746164;
    head->DataLen     = oSize;
    
    memcpy(tmpBuf, in_ptr, PCM_HEAD_LEN);

    *out_ptr_ptr = tmpBuf;
    return (oSize + PCM_HEAD_LEN); // need free out of the class;
}

void HanldeFfmpegResult::SplitChannel()
{
    if (CHANNEL_MONO == m_chanlNum)
    {
        m_len_1 = GetResampleResult(m_buf, m_len, &m_buf_1);
    }
    else if (CHANNEL_STEREO == m_chanlNum)
    {
        char  *tmp_buf_1 = NULL;
        size_t tmp_len_1 = (m_len + sizeof(PCM_HEAD) * (_16BITS_SHORT_SIZE - 1)) / _16BITS_SHORT_SIZE;
        char  *tmp_buf_2 = NULL;
        size_t tmp_len_2 = (m_len + sizeof(PCM_HEAD) * (_16BITS_SHORT_SIZE - 1)) / _16BITS_SHORT_SIZE;
        tmp_buf_1 = new char[tmp_len_1];
        tmp_buf_2 = new char[tmp_len_2];

        memcpy(tmp_buf_1, m_buf, sizeof(PCM_HEAD));
        memcpy(tmp_buf_2, m_buf, sizeof(PCM_HEAD));

        for (size_t i = gs_offset; i < m_len; i += _16BITS_SHORT_SIZE)
        {
            size_t tmp = gs_offset + ((i - gs_offset) >> 1);
            if (0 == (i & 0x03))  // channel 1
            {
                tmp_buf_1[tmp]     = m_buf[i];
                tmp_buf_1[tmp + 1] = m_buf[i + 1];
            }
            else                  // channel 2
            {
                tmp_buf_2[tmp - 1] = m_buf[i];
                tmp_buf_2[tmp]     = m_buf[i + 1];
            }
        }

        m_len_1 = GetResampleResult(tmp_buf_1, tmp_len_1, &m_buf_1);
        m_len_2 = GetResampleResult(tmp_buf_2, tmp_len_2, &m_buf_2);

        delete[] tmp_buf_1;
        tmp_buf_1 = NULL;
        delete[] tmp_buf_2;
        tmp_buf_2 = NULL;
    }
    else
    {
        // ERROR
    }
}

size_t HanldeFfmpegResult::SwapBuff(char *& buf)
{
    size_t len = 0;
    if (m_buf_1 != NULL)
    {
        buf     = m_buf_1;
        len     = m_len_1;
        m_buf_1 = NULL;
        m_len_1 = 0;
    }
    else if (m_buf_2 != NULL)
    {
        buf     = m_buf_2;
        len     = m_len_2;
        m_buf_2 = NULL;
        m_len_2 = 0;
    }
    else
    {
       buf = NULL;
       len = 0;
    }

    return len;
}



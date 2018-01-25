#ifndef _TIT_RESAMPLE_H_
#define _TIT_RESAMPLE_H_
#include <iostream>
#include <memory.h>

#include <soxr-lsr.h>

class TtsResample
{
public:
    TtsResample(double irate = 48000,
                double orate = 8000,
                SRC_SRCTYPE_e src_type = SRC_SINC_FASTEST);
    ~TtsResample();

    void SetIORate(double irate, double orate);
    void GetIORate(double &irate, double &orate);
    void ResampleBuf(short *data_in,  size_t inSize,
                     short *data_out, size_t outSize);
    
private:
    double          m_irate;
    double          m_orate;
    size_t          m_olen;
    SRC_DATA        m_data;
    SRC_SRCTYPE_e   m_src_type;
};

#endif//_TIT_RESAMPLE_H_

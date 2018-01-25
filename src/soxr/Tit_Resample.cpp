#include "Tit_Resample.h"

using namespace std;

TtsResample::TtsResample(double irate, double orate, SRC_SRCTYPE_e src_type)
    : m_irate(irate), m_orate(orate), m_olen(0), m_src_type(src_type)
{
    memset(&m_data, 0x00, sizeof(SRC_DATA));
}

TtsResample::~TtsResample()
{
}

void TtsResample::SetIORate(double irate, double orate)
{
    m_irate = irate;
    m_orate = orate;
}

void TtsResample::GetIORate(double &irate, double &orate)
{
    irate = m_irate;
    orate = m_orate;
}

void TtsResample::ResampleBuf(short *data_in,  size_t inSize,
                              short *data_out, size_t outSize)
{
    if (0 == m_irate || 0 == m_orate)
    {
        cout << "Error: Please set irate or orate!" << endl;
        return;
    }

    m_olen = (size_t)(inSize * m_orate / m_irate + .5);

    if (m_data.data_in != NULL || m_data.data_out != NULL)
    {
        delete m_data.data_in;
        m_data.data_in = NULL;
        delete m_data.data_out;
        m_data.data_out = NULL;
    }

    m_data.data_in       = new SRC_SAMPLE[inSize];
    m_data.data_out      = new SRC_SAMPLE[m_olen];
    m_data.input_frames  = static_cast<long>(inSize);
    m_data.output_frames = static_cast<long>(m_olen);
    m_data.src_ratio     = m_orate / m_irate;

    src_short_to_float_array(data_in, m_data.data_in, inSize);

    // 重采样 - zhangpeng
    SRC_ERROR error = src_simple(&m_data, m_src_type, 1);
    (void)error;
    if (m_data.input_frames_used != m_data.input_frames)
    {
        cout << "WARN: the data isn't resample complete!" << endl;
        cout << "m_data.input_frames_used " << m_data.input_frames_used << endl;
        cout << "m_data.input_frames " << m_data.input_frames << endl;
    }

    long outLen = (m_data.output_frames_gen > static_cast<long> (outSize)) ? \
                  static_cast<long> (outSize) : m_data.output_frames_gen;

    src_float_to_short_array(m_data.data_out, data_out, outLen);

    delete m_data.data_in;
    m_data.data_in = NULL;
    delete m_data.data_out;
    m_data.data_out = NULL;
}


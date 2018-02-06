#ifndef _TIT_PRASE_AUDIO_INFO_H_
#define _TIT_PRASE_AUDIO_INFO_H_
#include <string>
#include "common.h"
#include "Tit_Map.h"

class PraseAudioInfo
{
public:
    /* Audio: pcm_mulaw ([7][0][0][0] / 0x0007), 8000 Hz, 1 channels, s16, 64 kb/s
     * Audio: none ([4][161][0][0] / 0xA104), 8000 Hz, 2 channels, 16 kb/s
     * Audio: aac (mp4a / 0x6134706D), 44100 Hz, mono, fltp, 63 kb/s
     */
    PraseAudioInfo(std::string info);
    ~PraseAudioInfo();

    VoiceType    GetAudioType();
    ChannelNum   GetChannelNum();
    StereoOnMode GetStereoOnMode();        
    int GetSample();
    int GetBits();
    int GetSpeed();
private:
    std::string   m_info;
    std::string   m_audio;
    std::string   m_Hz;
    std::string   m_channels;
    int           m_adjust;
    std::string   m_bits;
    std::string   m_speed;
};

void InitAudioTypeMap();

#endif//_TIT_PRASE_AUDIO_INFO_H_

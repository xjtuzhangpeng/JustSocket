#include "PraseAudioInfo.h"

#define AUDIO_INFO_HEAD         "Audio:"
// 语音的类型
#define AUDIO_TYPE_PCM_MULAW    std::string("pcm_mulaw")
#define AUDIO_TYPE_PCM_aLAW     std::string("pcm_alaw")
#define AUDIO_TYPE_IMA_WAV      std::string("adpcm_ima_wav")
#define AUDIO_TYPE_ADPCM_MS     std::string("adpcm_ms")
#define AUDIO_TYPE_GSM_MS       std::string("gsm_ms")
#define AUDIO_TYPE_AAC          std::string("aac")
#define AUDIO_TYPE_TRUE_SPEECH  std::string("truespeech")
#define AUDIO_TYPE_G726         std::string("adpcm_g726")
#define AUDIO_TYPE_PCM_S16LE    std::string("pcm_s16le")
#define AUDIO_TYPE_PCM_U8       std::string("pcm_u8")
#define AUDIO_TYPE_MP3          std::string("mp3")
#define AUDIO_TYPE_NONE         std::string("none")

#define INDEX_OF_AUDIO_SPLIT_BY_COMMA       0
#define INDEX_OF_HZ_SPLIT_BY_COMMA          1
#define INDEX_OF_CHANNEL_SPLIT_BY_COMMA     2
#define INDEX_OF_BITS_SPLIT_BY_COMMA        3  // AUDIO_TYPE_NONE don't have bits info
#define INDEX_OF_SPEED_SPLIT_BY_COMMA       4  // for AUDIO_TYPE_NONE, it shoule be 3
#define ADJUST_OF_AUDIO_TYPE_NONE           -1

#define ONE_CHANNELS             " 1 channels"
#define TWO_CHANNELS             " 2 channels"
#define MONO_CHANNELS            " mono"
#define STEREO_CHANNELS          " stereo"

TIT_Map<std::string, VoiceType>  g_AduioType;

void InitAudioTypeMap()
{
    std::pair<std::string, VoiceType> tmp(AUDIO_TYPE_PCM_MULAW, acm);
    g_AduioType.insert(tmp);
    std::pair<std::string, VoiceType> tmp_mp3(AUDIO_TYPE_MP3, mp3);
    g_AduioType.insert(tmp_mp3); // mp3_8k
}

PraseAudioInfo::PraseAudioInfo(std::string info) : m_info(info)
{
    std::vector<std::string> vct;
    SplitString(m_info, vct, ",");

    m_audio    = vct[INDEX_OF_AUDIO_SPLIT_BY_COMMA];
    m_Hz       = vct[INDEX_OF_HZ_SPLIT_BY_COMMA];
    m_channels = vct[INDEX_OF_CHANNEL_SPLIT_BY_COMMA];
    if (m_audio.find_first_of("none") != std::string::npos)
    {
        m_bits   = vct[INDEX_OF_BITS_SPLIT_BY_COMMA];
        m_adjust = 0;
    }
    else
    {
        m_bits   = "";
        m_adjust = ADJUST_OF_AUDIO_TYPE_NONE;
    }
    m_speed  = vct[INDEX_OF_SPEED_SPLIT_BY_COMMA + m_adjust];
}

PraseAudioInfo::~PraseAudioInfo()
{
}

VoiceType PraseAudioInfo::GetAudioType()
{
    std::vector<std::string> vct;
    SplitString(m_audio, vct, " ");
    if (AUDIO_INFO_HEAD != vct[0] ||
        g_AduioType.find(vct[1]) == g_AduioType.end())
    {
         return sum;
    }

    return g_AduioType[vct[1]];
}

int PraseAudioInfo::GetSample()
{
    std::vector<std::string> vct;
    SplitString(m_audio, vct, " ");
    if ("Hz" != vct[1])
    {
         return 0;
    }
    return str2int(vct[0]);
}

ChannelNum PraseAudioInfo::GetChannelNum()
{
    if (m_channels == MONO_CHANNELS || m_channels == ONE_CHANNELS)
    {
        return CHANNEL_MONO;
    }
    else if (m_channels == STEREO_CHANNELS || m_channels == TWO_CHANNELS)
    {
        return CHANNEL_STEREO;
    }
    else
    {
        return CHANNEL_UNKNOW;
    }
}

StereoOnMode PraseAudioInfo::GetStereoOnMode()
{
    if (GetChannelNum() != CHANNEL_STEREO)
    {
        return NOT_STEREO;
    }
    return STEREO_ON_1;
}

int PraseAudioInfo::GetBits()
{
    if (m_bits == "")
    {
        return 0;
    }
    else if (m_bits == "fltp")
    {
        return -1;
    }

    std::string bits = m_bits.substr(1, std::string::npos);
    return str2int(bits);
}

int PraseAudioInfo::GetSpeed()
{
    return str2int(m_speed);
}



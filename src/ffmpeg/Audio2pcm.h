#ifndef _AUDIO_2_PCM_H_
#define _AUDIO_2_PCM_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>

#include "common.h"
#include "Tit_Logger.h"
#include "Tit_SocketInfo.h"
#include "Tit_AVIOReading.h"
#include "PraseAudioInfo.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

extern int SOX_main(int argc, char* argv[]);
extern int main_sox(int argc, char* argv[]);
extern int main_ffmpeg(int argc, char* argv[]);

#ifdef __cplusplus
}
#endif

#define FFMEPG_SOCKET_PORT      30020

#define FFMEPG_MML_HEAD         "ffmpeg -loglevel fatal "
#define FFMEPG_CONF_MONO        " -ac 1 "
#define FFMPEG_CONF_RESAMPLE    " -ar 8000 "
#define FFMPEG_CONF_BITRATE     " -ab 128k "
#define FFMPEG_CONG_FORMAT      " -f wav "
#define FFMPEG_TMP_OUTPUT       " /tmp/ffmpeg_out.wav "

#define SOX_MML_HEAD            "sox "
#define SOX_TMP_INPUT           string(" /tmp/sox_in.wav ")
#define SOX_TMP_OUTPUT          string(" /tmp/sox_out.wav ")

//转码相关参数
typedef struct DecodePara
{
	string       inWavName;     //输入语音（含路径）
	string       outPath;		//转码后语音存放路径
	ChannelNum   channel;	    //声道数
	VoiceType    trueFormat;	//输入语音编码格式
	StereoOnMode stereOnMode;	//双声道语音转码方式
	int          sessionId;     //线程id
}DecodePara;


class Audio2Pcm
{
public:
    Audio2Pcm(int threadNum);
    ~Audio2Pcm();

    //将语音转码为8k_16bit_pcm格式
    bool   audio2pcm(DecodePara *para);
    size_t GetAudioBuf(int sessionId, char **buf_ptr);
private:
    typedef struct _AudioBuf
    {
        DecodePara   *para;
        char         *buf_in;
        char         *buf_out;
        size_t        buf_in_size;
        size_t        buf_out_size;
        //单声道转码后语音名称：
        //outPath/inWavBaseName.wav,
        //双声道转为左右单声道语音，分别为：
        //outPath/inWavBaseName_left.wav,
        //outPath/inWavBaseName_right.wav,
        string        outWavName;
        string        outWavName_left;
        string        outWavName_right;

        _AudioBuf() : para(NULL), buf_in(NULL), buf_out(NULL), 
                     buf_in_size(0), buf_out_size(0), 
                     outWavName(""), outWavName_left(""), outWavName_right("")
        {
        }
        ~_AudioBuf()
        {
            Cleanup();
        }
        void Cleanup()
        {
            para             = NULL;
            delete buf_in;
            buf_in           = NULL;
            delete buf_out;
            buf_out          = NULL;
            buf_in_size      = 0;
            buf_out_size     = 0;
            outWavName       = "";
            outWavName_left  = "";
            outWavName_right = "";
        }
    }AudioBuf;

    void CallCodecFunction(int sessionId, string &cmd);
    void ParseAudioFormatInfo(int sessionId);
    void ReadInWavToAudioBuf(int sessionId);
    void SendBufToSox(AudioBuf *AuBufPtr, DecodePara *decodePara);
    void ExchangeBufOut2In(int sessionId);

    void HandleVoiceType_pcm(int sessionId);
    void HandleVoiceType_pcm_spec(int sessionId);
    void HandleVoiceType_vox(int sessionId);
    void HandleVoiceType_alaw(int sessionId);
    void HandleVoiceType_alaw_raw(int sessionId);
    void HandleVoiceType_acm_or_voc(int sessionId);
    void HandleVoiceType_mp3_or_8k(int sessionId);
    void HandleVoiceType_raw(int sessionId);
    void HandleVoiceType_ffmpeg_8kbps(int sessionId);
private:
    int                      m_threadNum;
    SocketInfo              *m_audioSocket;
    std::vector<AudioBuf *>  m_audioBuf;  // Get The Buf
};

#endif

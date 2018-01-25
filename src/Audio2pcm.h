#ifndef _AUDIO_2_PCM_H_
#define _AUDIO_2_PCM_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>

#include "Tit_Logger.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

extern int main_sox(int argc, char* argv[]);
extern int main_ffmpeg(int argc, char* argv[]);

#ifdef __cplusplus
}
#endif


//支持的语种格式
enum VoiceType
{
	sum=0,
	pcm,                 //128kbps的pcm: 8k_16bit_PCM
	pcm_spec,            //采样率为非8k的pcm: 6k_16bit_PCM
	vox,                 //6k_4bit的vox: 6k_4bit_Vox
	alaw,                //带头的alaw: 8k_8bit_Alaw
	alaw_raw,            //没头的alaw
	acm,                 //adpcm:8k_8bit_Alaw, 8k_8bit_Ulaw, 8k_8bit_PCM, 8k_16bit_13kbps_GSM 6.10, 8k_16bit_32kbps_ima-adpcm, 8k_16bit_32kbps_ms-adpcm, 
	voc,                 //标准的voc
	mp3,                 //MP3语音: 44100Hz_16bit_128kbps_mp3
	mp3_8k,              //8k_16bit_32kbps_mp3, 8k_16bit_8kbps_mp3
	raw,                 //没头的128kbps的pcm
	ffmpeg_8kbps,        //其他的可以用ffmpeg统一解码的压缩语音(对应徐晓艳8kbps): 8kbps,63kbps,16kbps
};

//语音声道数
enum ChannelNum
{
	CHANNEL_MONO=1,	//单声道
	CHANNEL_STEREO,		//双声道
};

//双声道语音转码方式
enum  StereoOnMode
{
	STEREO_ON_0=0,		//只将两个单声道语音合并成一个分录的双声道语音，并删除原来的两个单声道语音
	STEREO_ON_1,		//将分录的双声道语音合并成一个合录的单声道语音
	STEREO_ON_2,		//将分录双声道的左右声道解码成两个单声道
	STEREO_ON_3,		//将分录双声道的左右声道解码成两个单声道同时，还需合并成一个单声道
	STEREO_ON_4			//合录双声道，左右声道内容一样，都是多个人的对话，则仅保留一个声道内容
};

//转码相关参数
typedef struct DecodePara
{
	string inWavName;	//输入语音（含路径）
	string outPath;		//转码后语音存放路径
	ChannelNum channel;	//声道数
	VoiceType trueFormat;	//输入语音编码格式
	StereoOnMode stereOnMode;	//双声道语音转码方式
	
}DecodePara;


class Audio2Pcm
{
public:
    Audio2Pcm();
    ~Audio2Pcm();
    //将语音转码为8k_16bit_pcm格式
    bool audio2pcm(DecodePara *para);
    void CallCodecFunction(string &cmd);

    //单声道转码后语音名称：outPath/inWavBaseName.wav,
    //双声道转为左右单声道语音，分别为：outPath/inWavBaseName_left.wav和outPath/inWavBaseName_right.wav,
	string 			 inWavName;
    string           outWavName;
    string           outWavName_left;
    string           outWavName_right;
private:
    void HandleVoiceType_pcm();
    void HandleVoiceType_pcm_spec();
    void HandleVoiceType_vox();
    void HandleVoiceType_alaw();
    void HandleVoiceType_alaw_raw();
    void HandleVoiceType_acm_or_voc();
    void HandleVoiceType_mp3_or_8k();
    void HandleVoiceType_raw();
    void HandleVoiceType_ffmpeg_8kbps();
private:
	DecodePara	*decodePara;
};

#endif

#ifndef _TIT_COMMON_H_
#define _TIT_COMMON_H_
#include <stdlib.h>

#define TBNR_INPUT_SAMPLES       (8000)
#define PCM_HEAD_LEN             sizeof(PCM_HEAD)
#define CHAR_LEN_SHORT_LEN(in)   ((in) / 2)

typedef unsigned char   uint8;
typedef unsigned int    uint32;
typedef unsigned short  uint16;

// pcm文件头  
typedef struct
{  
    uint32 ChunkID;             //00H 4 char "RIFF"标志  
    uint32 ChunkSize;           //04H 4 long int 文件长度 文总长-8  
    uint32 Format;              //08H 4 char "WAVE"标志  
    uint32 SubChunk1ID;         //0CH 4 char "fmt "标志  
    uint32 SubChunk1Size;       //10H 4 0x10000000H(PCM)过渡字节(不定)  
    uint16 AudioFormat;         //14H 2 int 格式类别（0x01H为PCM形式的声音数据) 0x0100H  
    uint16 NumChannels;         //16H 2 int 通道数，单声道为1，双声道为2  
    uint32 SampleRate;          //18H 4 int 采样率（每秒样本数），表示每个通道的播放速度，  
    uint32 ByteRate;            //1CH 4 long int 波形音频数据传送速率，其值Channels×SamplesPerSec×BitsPerSample/8  
    uint16 BlockAlign;          //20H 2 int 数据块的调整数（按字节算的），其值为Channels×BitsPerSample/8  
    uint16 BitsPerSample;       //22H 2 每样本的数据位数，表示每个声道中各个样本的数据位数。如果有多个声道，对每个声道而言，样本大小都一样。  
    uint8  INFO[34];            //24H 34 char 描述信息
    uint32 DataTag;             //46H 4 char 数据标记符＂data＂  
    uint32 DataLen;             //4AH 4 long int 语音数据的长度(文长-44)  
} __attribute__((packed)) PCM_HEAD, *PPCM_HEAD;
  
// a-law文件头  
typedef struct  
{
    uint32 ChunkID;             //00H 4 char "RIFF"标志  
    uint32 ChunkSize;           //04H 4 long int 文件长度 文总长-8  
    uint32 Format;              //08H 4 char "WAVE"标志  
    uint32 SubChunk1ID;         //0CH 4 char "fmt "标志  
    uint32 SubChunk1Size;       //10H 4 0x12000000H(ALAW)  
    uint16 AudioFormat;         //14H 2 int 格式类别 0x0600H  
    uint16 NumChannels;         //16H 2 int 通道数，单声道为1，双声道为2  
    uint32 SampleRate;          //18H 4 int 采样率（每秒样本数），表示每个通道的播放速度，  
    uint32 ByteRate;            //1CH 4 long int 波形音频数据传送速率，其值Channels×SamplesPerSec×BitsPerSample/8  
    uint16 BlockAlign;          //20H 2 int 数据块的调整数（按字节算的），其值为Channels×BitsPerSample/8  
  //uint16 BitsPerSample;       //22H 2 每样本的数据位数，表示每个声道中各个样本的数据位数。如果有多个声道，对每个声道而言，样本大小都一样。  
    uint32 BitsPerSample;       //22H 2 每样本的数据位数，表示每个声道中各个样本的数据位数。如果有多个声道，对每个声道而言，样本大小都一样。  
    uint32 WaveFact;            //26H 4 char "fact"标志  
    uint32 Temp1;               //2AH 4 0x04000000H  
    uint32 Temp2;               //2EH 4 0x00530700H  
    uint32 DataTag;             //32H 4 char 数据标记符＂data＂  
    uint32 DataLen;             //36H 4 long int 语音数据的长度(文长-58)  
} __attribute__((packed)) ALAW_HEAD, *PALAW_HEAD;


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

#endif


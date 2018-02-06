#ifndef _AUDIO_2_PCM_H_
#define _AUDIO_2_PCM_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>

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
#define SOX_MML_HEAD            "sox "

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
/*
sox:      SoX v14.4.1

Usage summary: [gopts] [[fopts] infile]... [fopts] outfile [effect [effopt]]...

SPECIAL FILENAMES (infile, outfile):
-                        Pipe/redirect input/output (stdin/stdout); may need -t
-d, --default-device     Use the default audio device (where available)
-n, --null               Use the `null' file handler; e.g. with synth effect
-p, --sox-pipe           Alias for `-t sox -'

SPECIAL FILENAMES (infile only):
"|program [options] ..." Pipe input from external program (where supported)
http://server/file       Use the given URL as input file (where supported)

GLOBAL OPTIONS (gopts) (can be specified at any point before the first effect):
--buffer BYTES           Set the size of all processing buffers (default 8192)
--clobber                Don't prompt to overwrite output file (default)
--combine concatenate    Concatenate all input files (default for sox, rec)
--combine sequence       Sequence all input files (default for play)
-D, --no-dither          Don't dither automatically
--effects-file FILENAME  File containing effects and options
-G, --guard              Use temporary files to guard against clipping
-h, --help               Display version number and usage information
--help-effect NAME       Show usage of effect NAME, or NAME=all for all
--help-format NAME       Show info on format NAME, or NAME=all for all
--i, --info              Behave as soxi(1)
--input-buffer BYTES     Override the input buffer size (default: as --buffer)
--no-clobber             Prompt to overwrite output file
-m, --combine mix        Mix multiple input files (instead of concatenating)
--combine mix-power      Mix to equal power (instead of concatenating)
-M, --combine merge      Merge multiple input files (instead of concatenating)
--multi-threaded         Enable parallel effects channels processing
--norm                   Guard (see --guard) & normalise
--play-rate-arg ARG      Default `rate' argument for auto-resample with `play'
--plot gnuplot|octave    Generate script to plot response of filter effect
-q, --no-show-progress   Run in quiet mode; opposite of -S
--replay-gain track|album|off  Default: off (sox, rec), track (play)
-R                       Use default random numbers (same on each run of SoX)
-S, --show-progress      Display progress while processing audio data
--single-threaded        Disable parallel effects channels processing
--temp DIRECTORY         Specify the directory to use for temporary files
-T, --combine multiply   Multiply samples of corresponding channels from all
                         input files (instead of concatenating)
--version                Display version number of SoX and exit
-V[LEVEL]                Increment or set verbosity level (default 2); levels:
                           1: failure messages
                           2: warnings
                           3: details of processing
                           4-6: increasing levels of debug messages
FORMAT OPTIONS (fopts):
Input file format options need only be supplied for files that are headerless.
Output files will have the same format as the input file where possible and not
overriden by any of various means including providing output format options.

-v|--volume FACTOR       Input file volume adjustment factor (real number)
--ignore-length          Ignore input file length given in header; read to EOF
-t|--type FILETYPE       File type of audio
-e|--encoding ENCODING   Set encoding (ENCODING may be one of signed-integer,
                         unsigned-integer, floating-point, mu-law, a-law,
                         ima-adpcm, ms-adpcm, gsm-full-rate)
-b|--bits BITS           Encoded sample size in bits
-N|--reverse-nibbles     Encoded nibble-order
-X|--reverse-bits        Encoded bit-order
--endian little|big|swap Encoded byte-order; swap means opposite to default
-L/-B/-x                 Short options for the above
-c|--channels CHANNELS   Number of channels of audio data; e.g. 2 = stereo
-r|--rate RATE           Sample rate of audio
-C|--compression FACTOR  Compression factor for output format
--add-comment TEXT       Append output file comment
--comment TEXT           Specify comment text for the output file
--comment-file FILENAME  File containing comment text for the output file
--no-glob                Don't `glob' wildcard match the following filename

AUDIO FILE FORMATS: 
  8svx aif aifc aiff aiffc al amb au avr cdda cdr cvs cvsd cvu dat dvms f32 f4 f64 f8 
  fssd gsm gsrt hcom htk ima ircam la lpc lpc10 lu maud nist prc raw s1 s16 s2 s24 s3 
  s32 s4 s8 sb sf sl sln smp snd sndr sndt sou sox sph sw txw u1 u16 u2 u24 u3 u32 u4 
  u8 ub ul uw vms voc vox wav wavpcm wve xa
PLAYLIST FORMATS: m3u pls
AUDIO DEVICE DRIVERS: alsa oss ossdsp pulseaudio

EFFECTS: 
  allpass band bandpass bandreject bass bend biquad chorus channels compand contrast dcshift 
  deemph delay dither divide+ downsample earwax echo echos equalizer fade fir firfit+ flanger 
  gain highpass hilbert input# loudness lowpass mcompand mixer* noiseprof noisered norm oops 
  output# overdrive pad phaser pitch rate remix repeat reverb reverse riaa silence sinc spectrogram 
  speed splice stat stats stretch swap synth tempo treble tremolo trim upsample vad vol
  * Deprecated effect    + Experimental effect    # LibSoX-only effect
EFFECT OPTIONS (effopts): effect dependent; see --help-effect
*/
#endif

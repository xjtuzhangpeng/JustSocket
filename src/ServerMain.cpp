#include "ServerMain.h"

#define TCP_PORT          (30011)
#define MAX_TEST_NUM      (1)
#define AUDIO_NAME(j)     "../Audio/mp3/music/test_1-" + int2str(j) + ".mp3"
/* "../Audio/test-" + int2str(j) + ".mp3"
 * "../Audio/mp3/44100/4410016bit_128kbps-" + int2str(j) + ".mp3"
 * "../Audio/verint/XAGR1001_20160805144832_15732665238-" + int2str(j) + ".WAV"
 * "../Audio/mp3/music/test_1" + int2str(j) + ".mp3"
 * */

#define SID(j, i)   sid_head + int2str(j) + "-" + int2str(i);

SocketInfo    G_Server(TCP_PORT);
LOGGER        G_Log("./2_buff_ffmpeg.log", -1);

void printfHead(char *buf)
{
    PPCM_HEAD head = reinterpret_cast<PPCM_HEAD> (buf);
    LOG_PRINT_DEBUG("+++++++++++++++++++++++++++++++++++");
    LOG_PRINT_DEBUG("head->ChunkID        0x%x", head->ChunkID);
    LOG_PRINT_DEBUG("head->ChunkSize      0x%x", head->ChunkSize);
    LOG_PRINT_DEBUG("head->Format         0x%x", head->Format);
    LOG_PRINT_DEBUG("head->SubChunk1ID    0x%x", head->SubChunk1ID);
    LOG_PRINT_DEBUG("head->SubChunk1Size  0x%x", head->SubChunk1Size);
    LOG_PRINT_DEBUG("head->AudioFormat    0x%x", head->AudioFormat);
    LOG_PRINT_DEBUG("head->NumChannels    0x%x", head->NumChannels);
    LOG_PRINT_DEBUG("head->SampleRate     0x%x", head->SampleRate);
    LOG_PRINT_DEBUG("head->ByteRate       0x%x", head->ByteRate);
    LOG_PRINT_DEBUG("head->BlockAlign     0x%x", head->BlockAlign);
    LOG_PRINT_DEBUG("head->BitsPerSample  0x%x", head->BitsPerSample);
    LOG_PRINT_DEBUG("head->DataTag        0x%x", head->DataTag);
    LOG_PRINT_DEBUG("head->DataLen        0x%x", head->DataLen);
    LOG_PRINT_DEBUG("+++++++++++++++++++++++++++++++++++\n");
}

void *ResultThread(std::string sid_head, int j, int i)
{
    std::string sid      = sid_head;
    size_t      len      = 0;

    sid = SID(j, i);
    while ( static_cast<long>(len = G_Server.GetFFmpegBuffLen(sid)) < 0 )
    {
        usleep(WAIT_TASK_OR_RESULT);
    }

    char *buf = new char[len + 10];
    if (len != G_Server.GetFFmpegBuff(sid, buf, len + 10))
    {
        LOG_PRINT_ERROR("sid: %s, len = %lu %x", sid.c_str(), len, len);
    }
    else
    {
        LOG_PRINT_INFO("sid: %s, len = %lu %x", sid.c_str(), len, len);
    }

    PPCM_HEAD head = reinterpret_cast<PPCM_HEAD> (buf);
    printfHead(buf);

    if (head->BitsPerSample != 0x10)
    {
        //转编码位数;
        LOG_PRINT_DEBUG("BitsPerSample %d", head->BitsPerSample);
    }

    HanldeFfmpegResult handle(buf, len);

    delete[] buf;
    buf = NULL;

    int tmp = 0;
    while (0 != (len = handle.SwapBuff(buf)))
    {
        printfHead(buf);

        sid = AUDIO_NAME(j + 1) + "-" + int2str(i) + int2str(++tmp);
        LOG_PRINT_DEBUG("FileName: %s", sid.c_str());
        FILE *fp = fopen(sid.c_str(), "w+");
        fwrite(buf + 78, len, 1, fp);
        fclose(fp);

        delete[] buf;
        buf = NULL;
        len = 0;
    }

    return NULL;
#if 1
    // 开始降或升采样 
    size_t inSize = len - PCM_HEAD_LEN;
    size_t oSize  = (size_t)(inSize * TBNR_INPUT_SAMPLES / head->SampleRate + .5);

    char *inBuf  = buf + PCM_HEAD_LEN;
    char *tmpBuf = new char[oSize + PCM_HEAD_LEN];
    char *oBuf   = tmpBuf + PCM_HEAD_LEN;

    TtsResample resample(head->SampleRate, TBNR_INPUT_SAMPLES);
    resample.ResampleBuf(reinterpret_cast<short *> (inBuf), CHAR_LEN_SHORT_LEN(inSize),
                         reinterpret_cast<short *> (oBuf),  CHAR_LEN_SHORT_LEN(oSize));

    // 重新赋值语音头
    head->ByteRate   = (head->NumChannels * TBNR_INPUT_SAMPLES * head->BitsPerSample) >> 3;
    head->BlockAlign = (head->NumChannels * head->BitsPerSample) >> 3;
    head->SampleRate = TBNR_INPUT_SAMPLES;
    head->ChunkSize  = oSize + PCM_HEAD_LEN;
    head->DataLen    = oSize;
    
    memcpy(tmpBuf, buf, PCM_HEAD_LEN);
    
    sid = AUDIO_NAME(j + 1) + "-" + int2str(i);
    LOG_PRINT_DEBUG("FileName: %s", sid.c_str());
    FILE *fp = fopen(sid.c_str(), "w+");
    fwrite(tmpBuf, (oSize + PCM_HEAD_LEN), 1, fp);
    fclose(fp);
#endif

    delete[] buf;
    buf = NULL;
    delete[] tmpBuf;
    tmpBuf = NULL;
    return NULL;
}

void *TestThread(std::string sid_head, int j, int i)
{
    std::string sid      = sid_head;
    int         port     = TCP_PORT;

    std::string cmd_head = "ffmpeg -loglevel fatal -i " AUDIO_NAME(j + 1) " -f wav tcp://127.0.0.1:";
    std::string command  = cmd_head;
    command = cmd_head + int2str(port);

    sid = SID(j, i);
    LOG_PRINT_INFO("Port:%d, SID:%s, CMD:%s", port, sid.c_str(), command.c_str());
    G_Server.InsertOneTask(sid, command);

    ResultThread(sid_head, j, i);

    return NULL;
}

void task_thread(std::string sid_head, int j)
{
    for (int i = 0; i < MAX_TEST_NUM; i++)
    {
        TestThread(sid_head, j, i);
    }
}

void test_ffmepg()
{
    std::string sid_head = "Test-";

    LOG_PRINT_FATAL("main start ... ");

    std::thread thrd_0(std::bind(task_thread, sid_head, 0));
    std::thread thrd_1(std::bind(task_thread, sid_head, 1));
    std::thread thrd_2(std::bind(task_thread, sid_head, 2));
    std::thread thrd_3(std::bind(task_thread, sid_head, 3));
    std::thread thrd_4(std::bind(task_thread, sid_head, 4));
    std::thread thrd_5(std::bind(task_thread, sid_head, 5));
    std::thread thrd_6(std::bind(task_thread, sid_head, 6));
    std::thread thrd_7(std::bind(task_thread, sid_head, 7));
    std::thread thrd_8(std::bind(task_thread, sid_head, 8));
    std::thread thrd_9(std::bind(task_thread, sid_head, 9));

    thrd_0.join();
    thrd_1.join();
    thrd_2.join();
    thrd_3.join();
    thrd_4.join();
    thrd_5.join();
    thrd_6.join();
    thrd_7.join();
    thrd_8.join();
    thrd_9.join();
    LOG_PRINT_FATAL("main finish ... ");

}

#define PATH_HEAD "/home/zhangpeng/workspace/zhangpeng/Test/JustSocket/Audio"
#define MAX_SESSION_NUM 10
#define SESSION_INCREMENT(sid) ((sid++) % MAX_SESSION_NUM)
void TestCase_AudioFormat()
{
    InitSessionNum(MAX_SESSION_NUM);
    int sissionId = 0;

    AVIOReading(PATH_HEAD "/ACM/mu-law.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/8k/8k16bit_8kbps.mp3", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/8k/8k16bit_28kbps.mp3", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/8k/8k16bit_29kbps.mp3", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/8k/8k16bit_30kbps.mp3", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/44100/4410016bit_128kbps.mp3", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/4410016bit_128kbps.pcm", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/ACM/alaw.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/ACM/GSM_6.10_13kbps.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/ACM/mu-law.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/adpcm/32kbps_ima_adpcm.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/adpcm/32kbps_microsoft_adpcm.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/m4a/63kbps.m4a", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/other/8kbps.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/other/16kbps.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/pcm/6k16bit_96kbps.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/pcm/8k8bit_64kbps.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/pcm/000194_5s000216_5s000110_5s000232_5s000262_5s000271_5s000324_5s_8k16bit_pcm.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/pcm/0935584.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/verint/XAGR1001_20160805144832_15732665238.WAV", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/verint/XAGR1001_20160805150802_053115688888654.WAV", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/voc/14424390610008.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/voc/14432151710001.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/voc/14432152210004.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/voc/14440156310008.wav", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/music/test_1.MP3", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/music/test_2.MP3", SESSION_INCREMENT(sissionId));
    AVIOReading(PATH_HEAD "/mp3/music/test_3.MP3", SESSION_INCREMENT(sissionId));
    return;
}

void TestCase_Audio2Pcm()
{
    Audio2Pcm   audio2Pcm(MAX_NUM_OF_THREAD);
    DecodePara  para;

    para.inWavName   = PATH_HEAD "/opus/111.wav";
    para.outPath     = PATH_HEAD "/opus/";
    para.channel     = CHANNEL_MONO;
    para.trueFormat  = pcm_spec;
    para.stereOnMode = STEREO_ON_1;
    para.sessionId   = 1;

    audio2Pcm.audio2pcm(&para);

    char   *buf     = NULL;
    size_t  buf_len = audio2Pcm.GetAudioBuf(para.sessionId, &buf);
    if (buf_len == 0)
    {
        LOG_PRINT_DEBUG("buf is zero");
        return;
    }
    LOG_PRINT_DEBUG("buf-len %d", buf_len);

    std::string sid = "VOC_tmp_1.wav";
    LOG_PRINT_DEBUG("FileName: %s", sid.c_str());
    FILE *fp = fopen(sid.c_str(), "w+");
    fwrite(buf, buf_len, 1, fp);
    fclose(fp);
    buf = NULL;
}

int main(int agrs, char *argv[])
{
    std::string  filePath    = "/home/zhangpeng/workspace/zhangpeng/Test/JustSocket/Audio/mp3/";
    std::string  tempWavName = filePath + "4410016bit_128kbps";
    std::string  sox_cmd     = SOX_VOX_1((tempWavName + ".pcm"), (tempWavName + ".wav"));

    LOG_PRINT_DEBUG("PCM_HEAD = %d", sizeof(PCM_HEAD));
    //LOG_PRINT_DEBUG("Cmd: %s", sox_cmd.c_str());

    //test_ffmepg();
    //TestCase_AudioFormat();
    TestCase_Audio2Pcm();

    return 0;
}


#include "ServerMain.h"

#define TCP_PORT          (30011)
#define MAX_CONNECTED     (10)

#define MAX_TEST_NUM      (10)
#define AUDIO_NAME(j)     "../Audio/mp3/44100/4410016bit_128kbps-" + int2str(j) + ".mp3"
/* "../Audio/test-" + int2str(j) + ".mp3"
 * "../Audio/mp3/44100/4410016bit_128kbps-" + int2str(j) + ".mp3"
 * */

#define SID(j, i)   sid_head + int2str(j) + "-" + int2str(i);

SocketInfo    G_Server(TCP_PORT);
LOGGER        G_Log("./2_buff_ffmpeg.log");


void *ResultThread(std::string sid_head, int j, int i)
{
    std::string sid      = sid_head;
    size_t      len      = 0;

    sid = SID(j, i);
    while ( static_cast<long>(len = G_Server.GetBuffLen(sid)) < 0 )
    {
        usleep(WAIT_TASK_OR_RESULT);
    }

    char *buf = new char[len + 10];
    if (len != G_Server.GetBuff(sid, buf, len + 10))
    {
        LOG_PRINT_ERROR("sid: %s, len = %lu %x", sid.c_str(), len, len);
    }
    else
    {
        LOG_PRINT_INFO("sid: %s, len = %lu %x", sid.c_str(), len, len);
    }

    PPCM_HEAD head = reinterpret_cast<PPCM_HEAD> (buf);

    LOG_PRINT_DEBUG("1. 0x%x, 0x%x, 0x%x", head->ChunkID, head->ChunkSize, head->Format);
    LOG_PRINT_DEBUG("2. 0x%x, 0x%x, 0x%x", head->SubChunk1ID, head->SubChunk1Size, head->AudioFormat);
    LOG_PRINT_DEBUG("3. 0x%x, 0x%x, 0x%x", head->NumChannels, head->SampleRate, head->ByteRate);
    LOG_PRINT_DEBUG("4. 0x%x, 0x%x", head->BlockAlign, head->BitsPerSample);
    LOG_PRINT_DEBUG("5. 0x%x, 0x%x", head->DataTag, head->DataLen);

    if (head->BitsPerSample != 0x10)
    {
        //转编码位数;
        LOG_PRINT_DEBUG("BitsPerSample %d", head->BitsPerSample);
    }


    HanldeFfmpegResult handle(buf, len);

    delete[] buf;
    buf = NULL;

    while (0 != (len = handle.SwapBuff(buf)))
    {
        sid = AUDIO_NAME(j + 1) + "-" + int2str(i);
        LOG_PRINT_DEBUG("FileName: %s", sid.c_str());
        FILE *fp = fopen(sid.c_str(), "w+");
        fwrite(buf, len, 1, fp);
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
    /*std::thread thrd_1(std::bind(task_thread, sid_head, 1));
    std::thread thrd_2(std::bind(task_thread, sid_head, 2));
    std::thread thrd_3(std::bind(task_thread, sid_head, 3));
    std::thread thrd_4(std::bind(task_thread, sid_head, 4));
    std::thread thrd_5(std::bind(task_thread, sid_head, 5));
    std::thread thrd_6(std::bind(task_thread, sid_head, 6));
    std::thread thrd_7(std::bind(task_thread, sid_head, 7));
    std::thread thrd_8(std::bind(task_thread, sid_head, 8));
    std::thread thrd_9(std::bind(task_thread, sid_head, 9));*/

    thrd_0.join();
    /*thrd_1.join();
    thrd_2.join();
    thrd_3.join();
    thrd_4.join();
    thrd_5.join();
    thrd_6.join();
    thrd_7.join();
    thrd_8.join();
    thrd_9.join();*/
    LOG_PRINT_FATAL("main finish ... ");

}

int main(int agrs, char *argv[])
{
    std::string  filePath    = "/home/zhangpeng/workspace/zhangpeng/Test/JustSocket/Audio/mp3/";
    std::string  tempWavName = filePath + "4410016bit_128kbps";
    std::string  sox_cmd     = SOX_VOX_1((tempWavName + ".pcm"), (tempWavName + ".wav"));

    LOG_PRINT_DEBUG("PCM_HEAD = %d", sizeof(PCM_HEAD));
    LOG_PRINT_DEBUG("Cmd: %s", sox_cmd.c_str());

    test_ffmepg();

    Audio2Pcm audio;
    //audio.CallCodecFunction(sox_cmd);
    
    return 0;
}


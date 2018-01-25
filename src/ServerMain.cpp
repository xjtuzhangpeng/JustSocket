#include "ServerMain.h"

#define TCP_PORT          (30011)
#define MAX_CONNECTED     (10)

#define MAX_TEST_NUM      (100)
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
        LOG_PRINT_ERROR("sid: %s, len = %lu", sid.c_str(), len);
    }
    else
    {
        LOG_PRINT_INFO("sid: %s, len = %lu", sid.c_str(), len);
    }

    sid = AUDIO_NAME(j + 1) + "-" + int2str(i);
    FILE *fp = fopen(sid.c_str(), "w+");
    fwrite(buf, len, 1, fp);
    fclose(fp);

    delete[] buf;
    buf = NULL;

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

    /*std::thread thrd_0(std::bind(task_thread, sid_head, 0));
    std::thread thrd_1(std::bind(task_thread, sid_head, 1));
    std::thread thrd_2(std::bind(task_thread, sid_head, 2));
    std::thread thrd_3(std::bind(task_thread, sid_head, 3));
    std::thread thrd_4(std::bind(task_thread, sid_head, 4));
    std::thread thrd_5(std::bind(task_thread, sid_head, 5));
    std::thread thrd_6(std::bind(task_thread, sid_head, 6));
    std::thread thrd_7(std::bind(task_thread, sid_head, 7));
    std::thread thrd_8(std::bind(task_thread, sid_head, 8));
    std::thread thrd_9(std::bind(task_thread, sid_head, 9));*/

    /*thrd_0.join();
    thrd_1.join();
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
    std::string  sox_cmd = "sox --i";
    Audio2Pcm audio;
    audio.CallCodecFunction(sox_cmd);
    
    return 0;
}


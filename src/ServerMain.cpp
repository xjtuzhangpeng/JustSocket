#include "ServerMain.h"

#define TCP_PORT          (30011)
#define MAX_CONNECTED     (10)

#define MAX_TEST_NUM      (2)
#define AUDIO_NAME(j)     "../Audio/test-" + int2str(j) + ".mp3"
//#define AUDIO_NAME        "../Audio/mp3/44100/4410016bit_128kbps.mp3"

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
        LOG_PRINT_WARN("ERROR : %s, len = %lu", sid.c_str(), len);
    }
    else
    {
        LOG_PRINT_WARN("NORMAL: %s, len = %lu", sid.c_str(), len);
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

    std::string cmd_head = "ffmpeg -i " AUDIO_NAME(j + 1) " -f wav tcp://127.0.0.1:";
    std::string command  = cmd_head;
    command = cmd_head + int2str(port);

    sid = SID(j, i);
    LOG_PRINT_WARN("Port:%d, SID:%s, CMD:%s", port, sid.c_str(), command.c_str());
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

int main(int agrs, char *argv[])
{
    std::string sid_head = "Test-";

    std::thread thrd_0(std::bind(task_thread, sid_head, 0));
    std::thread thrd_1(std::bind(task_thread, sid_head, 1));
    std::thread thrd_2(std::bind(task_thread, sid_head, 2));

    thrd_0.join();
    thrd_1.join();
    thrd_2.join();

    return 0;
}


#include "ServerMain.h"

#define TCP_PORT          (30011)
#define MAX_CONNECTED     (10)

#define MAX_TEST_NUM      (100)
#define AUDIO_NAME(j)     "../Audio/test-" + int2str(j) + ".mp3"
//#define AUDIO_NAME        "../Audio/mp3/44100/4410016bit_128kbps.mp3"

#define SID(j, i)   sid_head + int2str(j) + "-" + int2str(i);

SocketInfo   *G_Server[MAX_CONNECTED];
std::thread  *G_Thread[MAX_CONNECTED];
LOGGER        G_Log;

void TestThread(std::string sid_head, int j)
{
    std::string sid      = sid_head;
    int         port     = TCP_PORT + j;
    size_t      len      = 0;

    std::string cmd_head = "ffmpeg -i " AUDIO_NAME(j + 1) " -f wav tcp://127.0.0.1:";
    std::string command  = cmd_head;
    command = cmd_head + int2str(port);

    for (int i = 0; i < MAX_TEST_NUM; i++)
    {
        sid = SID(j, i);

        LOG_PRINT_WARN("Port:%d, SID:%s, CMD:%s", port, sid.c_str(), command.c_str());
        G_Server[j]->InsertOneTask(sid, command);

        while ( static_cast<long>(len = G_Server[j]->GetBuffLen(sid)) < 0 )
        {
            usleep(WAIT_TASK_OR_RESULT);
        }
        char *buf = new char[len + 10];

        if (len != G_Server[j]->GetBuff(sid, buf, len + 10))
        {
            LOG_PRINT_WARN("ERROR:  len = %lu ", len);
        }
        else
        {
            LOG_PRINT_WARN("NORMAL: len = %lu", len);
        }

        sid = AUDIO_NAME(j + 1) + "-" + int2str(i);
        FILE *fp = fopen(sid.c_str(), "w+");
        fwrite(buf, len, 1, fp);
        fclose(fp);

        delete[] buf;
        buf = NULL;
    }
    return;
}

int main(int agrs, char *argv[])
{
    std::string sid_head = "Test-";
    std::string sid      = sid_head;

    // Init Server 
    for (int j = 0; j < MAX_CONNECTED; j++)
    {
        int port = TCP_PORT + j;
        G_Server[j] = new SocketInfo(port);
        G_Thread[j] = new std::thread(std::bind(&TestThread, sid_head, j));
    }

    for (int j = 0; j < MAX_CONNECTED; j++)
    {
        G_Thread[j]->join();
    }

    // Release the Server resource ...
    for (int j = 0; j < MAX_CONNECTED; j++)
    {
        delete G_Server[j];
        G_Server[j] = NULL;
    }

    return 0;
}


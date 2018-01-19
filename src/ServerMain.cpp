#include "ServerMain.h"

#define TCP_PORT          (18080)
#define MAX_CONNECTED     (1)

#define MAX_TEST_NUM      (100)
#define AUDIO_NAME        "../Audio/test.mp3"
//#define AUDIO_NAME        "../Audio/mp3/44100/4410016bit_128kbps.mp3"

SocketInfo g_Server(TCP_PORT);

int main(int agrs, char *argv[])
{
    std::string sid_head = "Test-";
    std::string sid      = sid_head;
    std::string command  = "ffmpeg -i " AUDIO_NAME " -f wav tcp://127.0.0.1:";
    command += int2str(TCP_PORT);

    std::cout << command.c_str() << std::endl;

    for (int i = 0; i < MAX_TEST_NUM; i++)
    {
        sid = sid_head + int2str(i);
        g_Server.InsertOneTask(sid, command);
    }

    size_t len = 0;
    sid = sid_head;
    for (int i = 0; i < MAX_TEST_NUM; i++)
    {
        sid = sid_head + int2str(i);
        while ( static_cast<long>(len = g_Server.GetBuffLen(sid)) < 0 )
        {
            usleep(WAIT_TASK_OR_RESULT);
        }
        char *buf = new char[len + 10];

        if (len != g_Server.GetBuff(sid, buf, len + 10))
        {
            printf("ERROR: len = %lu \n", len);
        }
        else
        {
            printf("NORMAL: len = %lu \n", len);
        }

        //sid = AUDIO_NAME "-" + sid;
        //FILE *fp = fopen(sid.c_str(), "w+");
        //fwrite(buf, len, 1, fp);
        //fclose(fp);

        delete[] buf;
        buf = NULL;
    }

    sleep(1000);
    return 0;
}


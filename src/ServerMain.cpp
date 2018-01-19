#include "ServerMain.h"

#define TCP_PORT          (18080)
#define MAX_CONNECTED     (1)
#define MY_MAX_BUFF_LEN   (300 * 1024 * 1024)

#define MAX_TEST_NUM      (100)

SocketInfo g_Server(TCP_PORT);

int main(int agrs, char *argv[])
{
    std::string sid_head = "0123456789-";
    std::string sid      = sid_head;
    std::string command  = "ffmpeg -i Audio/test.mp3 -f wav tcp://127.0.0.1:";
    command += int2str(TCP_PORT);

    std::cout << command.c_str() << std::endl;

    for (int i = 0; i < MAX_TEST_NUM; i++)
    {
        sid = sid_head + int2str(i);
        g_Server.InsertOneTask(sid, command);
    }

    sid        = sid_head;
    char  *buf = new char[MY_MAX_BUFF_LEN];
    size_t len = 0;

    for (int i = 0; i < MAX_TEST_NUM; i++)
    {
        sid = sid_head + int2str(i);
        while ( !(len = g_Server.GetBuff(sid, buf, MY_MAX_BUFF_LEN)) )
        {
            usleep(10);
        }
        printf("len = %lu \n", len);
    }

    return 0;
}


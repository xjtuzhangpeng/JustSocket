#include "ServerMain.h"

#define TCP_PORT          (18080)
#define MAX_CONNECTED     (20)

SocketInfo g_Server(TCP_PORT);

int main(int agrs, char *argv[])
{
    std::string sid_head = "0123456789-";
    std::string sid      = sid_head;
    std::string command  = "ffmpeg -i Audio/test.mp3 -f wav tcp://127.0.0.1:";
    command += int2str(TCP_PORT);

    std::cout << command.c_str() << std::endl;

    for (int i = 0; i < 10; i++)
    {
        sid += int2str(i);
        g_Server.InsertOneTask(sid, command);
    }

    sleep(10);
    sid        = sid_head;
    char  *buf = new char[MAX_BUFF_LEN];
    size_t len = 0;

    for (int i = 0; i < 10; i++)
    {
        sid += int2str(i);
        while ( !(len = g_Server.GetBuff(sid, buf, MAX_BUFF_LEN)) )
        {
            usleep(10);
        }
        printf("len = %lu \n", len);
    }

    sleep(100000);
    return 0;
}


#include "ServerMain.h"

#define TCP_PORT          (18080)
#define MAX_CONNECTED     (1)
#define MY_MAX_BUFF_LEN   (300 * 1024 * 1024)

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

    sid        = sid_head;
    char  *buf = new char[MY_MAX_BUFF_LEN];
    size_t len = 0;

    for (int i = 0; i < MAX_TEST_NUM; i++)
    {
        sid = sid_head + int2str(i);
        while ( !(len = g_Server.GetBuff(sid, buf, MY_MAX_BUFF_LEN)) )
        {
            usleep(WAIT_TASK_OR_RESULT);
        }
        printf("len = %lu \n", len);
        //sid = AUDIO_NAME "-" + sid;
        //FILE *fp = fopen(sid.c_str(), "w+");
        //fwrite(buf, len, 1, fp);
        //fclose(fp);
    }

    sleep(1000);

    return 0;
}


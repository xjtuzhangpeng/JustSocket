#include "ServerMain.h"

#define TCP_PORT          (65001)
#define MAX_CONNECTED     (10)

#define MAX_TEST_NUM      (100)
#define AUDIO_NAME        "../Audio/test.mp3"
//#define AUDIO_NAME        "../Audio/mp3/44100/4410016bit_128kbps.mp3"

#define SID(j, i)   sid_head + int2str(j) + "-" + int2str(i);

SocketInfo *G_Server[MAX_CONNECTED];

int main(int agrs, char *argv[])
{
    std::string sid_head = "Test-";
    std::string sid      = sid_head;
    std::string cmd_head = "ffmpeg -i " AUDIO_NAME " -f wav tcp://127.0.0.1:";
    std::string command  = cmd_head;

    // Init Server 
    for (int j = 0; j < MAX_CONNECTED; j++)
    {
        int port = TCP_PORT + j;
        G_Server[j] = new SocketInfo(port);
    }

    std::cout << "Server Start OK ....." << std::endl;
    sleep(5);

    // Set task ...
    for (int j = 0; j < MAX_CONNECTED; j++)
    {
        int port = TCP_PORT + j;
        command = cmd_head + int2str(port);
        for (int i = 0; i < MAX_TEST_NUM; i++)
        {
            sid = SID(j, i);

            std::cout << port << " CMD: " << command << std::endl;
            std::cout << port << " SID: " << sid << std::endl;
            G_Server[j]->InsertOneTask(sid, command);
        }
    }

    size_t len = 0;
    sid = sid_head;
    // Get result ...
    for (int j = 0; j < MAX_CONNECTED; j++)
    {
        for (int i = 0; i < MAX_TEST_NUM; i++)
        {
            sid = SID(j, i);
            while ( static_cast<long>(len = G_Server[j]->GetBuffLen(sid)) < 0 )
            {
                usleep(WAIT_TASK_OR_RESULT);
            }
            char *buf = new char[len + 10];

            if (len != G_Server[j]->GetBuff(sid, buf, len + 10))
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
    }

    // Release the Server resource ...
    for (int j = 0; j < MAX_CONNECTED; j++)
    {
        delete G_Server[j];
        G_Server[j] = NULL;
    }

    return 0;
}


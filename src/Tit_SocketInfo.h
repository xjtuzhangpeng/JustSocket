#ifndef _TIT_SOCKET_INFO_H_
#define _TIT_SOCKET_INFO_H_
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

#include "CPPTcpSocket.h"
#include "CPPUdpSocket.h"

#include "Tit_Map.h"
#include "Tit_TaskInfo.h"
#include "Audio2pcm.h"

#define WAIT_TASK_OR_RESULT  1000   // us

class SocketInfo
{
public:
    SocketInfo(short port
#ifdef _NODE_LINK_
#else
               , size_t buf_len = MAX_BUFF_LEN
#endif
               );
	~SocketInfo();

    size_t GetBuff(std::string sessionId, char *buf, size_t buff_len);
    void   InsertOneTask(std::string &sessionId, std::string &command);

private:
    void        StartServer(short port);
    bool        StartSendTask();
    void        SetLastTask(std::string sessionId);
    std::string GetLastTask();
    void        ReceiveData();
    std::string GetTask();
    bool        IsEmpty();
    std::string FrontTask();

    void PopTask();
    void InsertTask(std::string& sessionId);
    void ThreadDetach(std::string command);

#ifdef _NODE_LINK_
#else
    size_t                    m_buf_len;
#endif
    CPPSocket                *m_socket;

	std::mutex                m_mutex;
    std::string               m_sessionId;

    std::condition_variable   m_wait_cv;
    std::mutex                m_task_mutex;
    std::queue<std::string>   m_task_sessionId;

    TIT_Map<TaskInfo *>       m_task_map;
    Audio2Pcm                 m_Audio2Pcm;
};

#endif

#ifndef _TIT_SOCKET_INFO_H_
#define _TIT_SOCKET_INFO_H_
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>

#include "Tit_Map.h"
#include "Tit_TaskInfo.h"

class SocketInfo
{
public:
    SocketInfo(short port, size_t buf_len = MAX_BUFF_LEN) : m_buf_len(buf_len);
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

    size_t                    m_buf_len;
    CPPSocket                *m_socket;
    std::mutex                m_mutex;
    std::string               m_sessionId;

    std::mutex                m_task_mutex;
    std::queue<std::string>   m_task_sessionId;

    TIT_Map<TaskInfo *>       m_task_map;
};

#endif

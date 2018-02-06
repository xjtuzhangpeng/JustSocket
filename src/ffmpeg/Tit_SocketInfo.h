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

#define WAIT_TASK_OR_RESULT     1000   // us
#define MAX_NUM_OF_THREAD       10
#define MAX_NUM_OF_CONNECT      (MAX_NUM_OF_THREAD * 2)

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

    size_t GetFFmpegBuff(std::string sessionId, char *buf, size_t buff_len);
    size_t GetFFmpegBuffLen(std::string &sessionId);
    void   InsertOneTask(std::string &sessionId, std::string &command);

private:
    void        StartServer(short port);
    void        StartSendTask();
    void        SetLastTask(std::string sessionId);
    std::string GetLastTask();
    void        ReceiveData(int fd, std::string last_task);
    std::string GetTask();
    bool        IsEmpty();
    std::string FrontTask();
    void        PlusWorkThread();
    void        MinusWorkThread();
    size_t      GetWorkThreadNum();

    void InsertTask(std::string& sessionId);
    void ThreadDetach(std::string command);

#ifdef _NODE_LINK_
#else
    size_t                    m_buf_len;
#endif
    std::mutex                m_mutex;
    std::string               m_sessionId;

    std::mutex                m_taskbgn_mutex;
    std::condition_variable   m_taskbgn_cv;        // 同步: 任务队列为空与接收到新的任务
    std::mutex                m_taskend_mutex;
    std::condition_variable   m_taskend_cv;        // 同步: 线程数阈值与socket接收数据结束
    std::mutex                m_socket_mutex;
    std::condition_variable   m_socket_cv;         // 同步: 开始转码与socket连接成功

    std::mutex                m_task_mutex;
    std::queue<std::string>   m_task_sessionId;

    std::mutex                m_thread_mutex;
    size_t                    m_thread_num;
    TIT_Map<std::string, TaskInfo *> m_task_map;
};

#endif

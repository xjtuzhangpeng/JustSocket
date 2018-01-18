#ifndef _SERVER_MAIN_H_
#define _SERVER_MAIN_H_
#include <thread>
#include <functional>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <queue>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "CPPTcpSocket.h"
#include "CPPUdpSocket.h"

#define MAX_BUFF_LEN       (1 * 1024 * 1024)

//////////////////////////////////MAP///////////////////////////////////////

template <typename T>
class TIT_Map{
public:
    TIT_Map(){}
    ~TIT_Map(){}

    void insert(typename std::pair<std::string, T> in)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        _tit_map.insert(in);
        return;
    }

    typename std::map<std::string, T>::iterator find(const std::string& sid)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.find(sid);
    }

    typename std::map<std::string, T>::iterator erase(typename std::map<std::string, T>::iterator it)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.erase(it);
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.empty();
    }

    typename std::map<std::string, T>::iterator begin()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.begin();
    }

    typename std::map<std::string, T>::iterator end()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.end();
    }

    T& operator[] (std::string& key)
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map[key];
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lock_tmp(_mutex);
        return _tit_map.size();
    }

private:
    typename std::map<std::string, T> _tit_map;
    std::mutex                        _mutex;
};

static inline std::string int2str(const int &input)
{
    std::string        output;
    std::ostringstream oStream;

    oStream << input;
    output = oStream.str();

    return output;
}

class BuffLink
{
public:
    ;

private:
    struct BuffNode
    {
        char             *;
        struct BuffNode  *next;
    };
};

class TaskInfo
{
public:
    TaskInfo(std::string& command, size_t bufflen = MAX_BUFF_LEN) :
        m_command(command), m_buff_len(bufflen), m_offset(0), m_socket_closed(false)
    {
        m_buff = new char[m_buff_len];
    }

    ~TaskInfo()
    {
        delete m_buff;
    }

    void SetSocketClosed(bool closed)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_socket_closed = closed;
    }

    bool IsSocketClosed()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_socket_closed;
    }

    std::string GetCommand()
    {
        return m_command;
    }

//private:
    std::string    m_command;
    char          *m_buff;
    size_t         m_buff_len;
	size_t         m_offset;

    std::mutex     m_mutex;
    bool           m_socket_closed;
};


/////////////////////////////////TCP SOCKET//////////////////////////////////////////

class SocketInfo
{
public:
    SocketInfo(short port, size_t buf_len = MAX_BUFF_LEN) : m_buf_len(buf_len)
	{
        std::thread thrd_server(std::bind(&SocketInfo::StartServer, this, port));
        thrd_server.detach();
        
        std::thread thrd_sender(std::bind(&SocketInfo::StartSendTask, this));
        thrd_sender.detach();
	}

	~SocketInfo()
	{
	}

    size_t GetBuff(std::string sessionId, char *buf, size_t buff_len)
	{
	    size_t    len  = 0;
        TaskInfo *task = m_task_map[sessionId];

        if (task == NULL)
        {
            return len;
        }

        if (task->IsSocketClosed())
        {
            len = (buff_len > task->m_offset) ? task->m_offset : buff_len;
            memcpy(buf, task->m_buff, len);
            m_task_map.erase( m_task_map.find(sessionId) );
            delete task;
            task = NULL;
        }

		return len;
	}

    void InsertOneTask(std::string &sessionId, std::string &command)
    {
        TaskInfo *tmp = new TaskInfo(command, m_buf_len);

        m_task_map.insert(std::make_pair(sessionId, tmp));

        InsertTask(sessionId);
        return;
    }

private:
    void StartServer(short port)
    {
        CPPTcpServerSocket server_tcp;
        server_tcp.listen(port);
    
        while (true)
        {
            int fd = server_tcp.accept(-1);
            m_socket = new CPPSocket(fd);
            ReceiveData();
            delete m_socket;
            m_socket = NULL;
        }
    }

    bool StartSendTask()
    {
        TaskInfo     *tmp         = NULL;
        std::string   session_now = "";
        while (true)
        {
            std::string session_tmp = GetTask();

            if (session_tmp != session_now)
            {
                session_now = session_tmp;
                SetLastTask(session_now);

                printf("StartSendTask: %s \n", GetLastTask().c_str());
                tmp = m_task_map[session_now];
                if (tmp == NULL)
                {
                    PopTask();
                    continue;
                }
                std::thread thrd(std::bind(&SocketInfo::ThreadDetach, this, tmp->m_command));
                thrd.detach();
            }
            else
            {
                usleep(10);
                continue;
            }
            tmp = NULL;
        }
        return true;
    }

    void SetLastTask(std::string sessionId)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessionId = sessionId;
        return;
    }

    std::string GetLastTask()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_sessionId;
    }

    void ReceiveData()
    {
        int          recv_ln   = 0;
        char        *buf       = NULL;
        size_t       buf_len   = 0;
        std::string  last_task = GetLastTask();

        if (last_task.empty())
        {
            printf ("task: %s \n", last_task.c_str());
            return;
        }

        TaskInfo *task = m_task_map[last_task];
        if (m_socket == NULL || task == NULL)
        {
            printf ("m_socket, task: %p , %p, %s \n", m_socket, task, last_task.c_str());
            return;
        }

        while (true)
        {
            m_socket->hasData(-1);
            buf     = task->m_buff + task->m_offset;
            buf_len = task->m_buff_len - task->m_offset;
            recv_ln = m_socket->recv(buf, buf_len, 0);

            task->m_offset += recv_ln;
            printf("m_offset %d, recv_ln %d \n", task->m_offset, recv_ln);
            if (recv_ln <= 0 || task->m_offset == task->m_buff_len)
            {
                task->m_offset -= recv_ln;
                task->SetSocketClosed(true);
                m_socket->close();
                PopTask(); // delete the last task;
                break;
            }
        }
        return;
    }

    std::string GetTask()
    {
        // 等待任务
        while (IsEmpty())
        {
            usleep(10);
            continue;
        }
        return FrontTask();
    }

    bool IsEmpty()
    {
        std::lock_guard<std::mutex> lock(m_task_mutex);
        return m_task_sessionId.empty();
    }

    std::string FrontTask()
    {
        std::lock_guard<std::mutex> lock(m_task_mutex);
        return m_task_sessionId.front();
    }

    void PopTask()
    {
        std::lock_guard<std::mutex> lock(m_task_mutex);
        printf("Task %s done. \n", m_task_sessionId.front().c_str());
        SetLastTask("");
        m_task_sessionId.pop();
    }

    void InsertTask(std::string& sessionId)
    {
        std::lock_guard<std::mutex> lock(m_task_mutex);
        m_task_sessionId.push(sessionId);
    }

    void ThreadDetach(std::string command)
    {
        // send task ... 
        system(command.c_str());
    }

    size_t                    m_buf_len;
    CPPSocket                *m_socket;
    std::mutex                m_mutex;
    std::string               m_sessionId;

    std::mutex                m_task_mutex;
    std::queue<std::string>   m_task_sessionId;

    TIT_Map<TaskInfo *>       m_task_map;
};

#endif//_SERVER_MAIN_H_

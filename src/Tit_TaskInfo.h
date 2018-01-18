#ifndef _TIT_TASK_INFO_H_
#define _TIT_TASK_INFO_H_
#include <string>

class TaskInfo
{
public:
    TaskInfo(std::string& command
#ifdef _NODE_LINK_
#else
        , size_t bufflen = MAX_BUFF_LEN
#endif
        ) :
#ifdef _NODE_LINK_
#else
        m_buff_len(bufflen),
        m_offset(0),
#endif
        m_command(command),
        m_socket_closed(false)
    {
#ifdef _NODE_LINK_
#else
        m_buff = new char[m_buff_len];
#endif
    }

    ~TaskInfo()
    {
#ifdef _NODE_LINK_
#else
        delete m_buff;
#endif
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

#ifdef _NODE_LINK_
    BuffLink       m_buff_link;
#else
    char          *m_buff;
    size_t         m_buff_len;
    size_t         m_offset;
#endif
    std::string    m_command;
    std::mutex     m_mutex;
    bool           m_socket_closed;
};

#endif//_TIT_TASK_INFO_H_
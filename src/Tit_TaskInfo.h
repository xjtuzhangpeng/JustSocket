#ifndef _TIT_TASK_INFO_H_
#define _TIT_TASK_INFO_H_
#include <string>
#include <mutex>

#include "Tit_BuffLink.h"

class TaskInfo
{
public:
    TaskInfo(std::string& command
#ifdef _NODE_LINK_
#else
        , size_t bufflen = MAX_BUFF_LEN
#endif
        );

    ~TaskInfo();

    void        SetSocketClosed(bool closed);
    bool        IsSocketClosed();
    std::string GetCommand();

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
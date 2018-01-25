#include "Tit_TaskInfo.h"

TaskInfo::TaskInfo(std::string& command
#ifdef _NODE_LINK_
#else
                   , size_t bufflen
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

TaskInfo::~TaskInfo()
{
#ifdef _NODE_LINK_
#else
    delete m_buff;
#endif
}

void TaskInfo::SetSocketClosed(bool closed)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_socket_closed = closed;
}

bool TaskInfo::IsSocketClosed()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_socket_closed;
}

std::string TaskInfo::GetCommand()
{
    return m_command;
}

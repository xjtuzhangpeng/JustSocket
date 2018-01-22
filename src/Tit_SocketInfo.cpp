#include "Tit_SocketInfo.h"

SocketInfo::SocketInfo(short port
#ifdef _NODE_LINK_
#else
                       , size_t buf_len
#endif
                       )
#ifdef _NODE_LINK_
#else
                       : m_buf_len(buf_len)
#endif
{
    std::thread thrd_server(std::bind(&SocketInfo::StartServer, this, port));
    thrd_server.detach();
    
    std::thread thrd_sender(std::bind(&SocketInfo::StartSendTask, this));
    thrd_sender.detach();
}

SocketInfo::~SocketInfo()
{
}

size_t SocketInfo::GetBuffLen(std::string &sessionId)
{
    size_t    len  = -1;
    TaskInfo *task = m_task_map[sessionId];

    if (task == NULL)
    {
        return len;
    }

    if (task->IsSocketClosed())
    {
#ifdef _NODE_LINK_
        len = task->m_buff_link.BuffLen();
#else
        len = task->m_offset;
#endif
    }

    return len;
}

size_t SocketInfo::GetBuff(std::string sessionId, char *buf, size_t buff_len)
{
    size_t    len  = 0;
    TaskInfo *task = m_task_map[sessionId];

    if (task == NULL)
    {
        return len;
    }

    if (task->IsSocketClosed())
    {
#ifdef _NODE_LINK_
        len = task->m_buff_link.CopyBuff(buf, buff_len);
#else
        len = (buff_len > task->m_offset) ? task->m_offset : buff_len;
        memcpy(buf, task->m_buff, len);
#endif
        m_task_map.erase( m_task_map.find(sessionId) );
        delete task;
        task = NULL;
    }

	return len;
}

void SocketInfo::InsertOneTask(std::string &sessionId, std::string &command)
{
    if (!sessionId.empty() && !command.empty())
    {
        TaskInfo *tmp = new TaskInfo(command
#ifdef _NODE_LINK_
#else
                                     , m_buf_len
#endif
                                     );
        m_task_map.insert(std::make_pair(sessionId, tmp));

        InsertTask(sessionId);
    }
    m_wait_cv.notify_one(); // notify handle a new task;
    return;
}

void SocketInfo::StartServer(short port)
{
    CPPTcpServerSocket server_tcp;
    server_tcp.listen(port);

    while (true)
    {
        LOG_PRINT_WARN("Wait accept: %d ...", port);
        int fd = server_tcp.accept(-1);
        LOG_PRINT_WARN("Accepted:    %d ...", port);
        m_socket = new CPPSocket(fd);
        ReceiveData();
        delete m_socket;
        m_socket = NULL;
    }
}

void SocketInfo::ReceiveData()
{
    int          recv_ln   = 0;
    char        *buf       = NULL;
    size_t       buf_len   = 0;
    std::string  last_task = GetLastTask();

    if (last_task.empty())
    {
        LOG_PRINT_WARN("Error: task: %s", last_task.c_str());
        return;
    }

    TaskInfo *task = m_task_map[last_task];
    if (m_socket == NULL || task == NULL)
    {
        LOG_PRINT_WARN("Error: m_socket, task: %p , %p, %s", m_socket, task, last_task.c_str());
        return;
    }

    while (true)
    {
#ifdef _NODE_LINK_
        struct BuffNode * node = task->m_buff_link.GetTail();
        buf     = node->buff + node->offset;
        buf_len = BUFF_NODE_LEN - node->offset;
#else
        buf     = task->m_buff + task->m_offset;
        buf_len = task->m_buff_len - task->m_offset;
#endif
        m_socket->hasData(-1);
        recv_ln = m_socket->recv(buf, buf_len, 0);
#ifdef _NODE_LINK_
        node->offset   += recv_ln;
        //LOG_PRINT_WARN("m_offset %lu, recv_ln %d", node->offset, recv_ln);
#else
        task->m_offset += recv_ln;
        //LOG_PRINT_WARN("m_offset %lu, recv_ln %d", task->m_offset, recv_ln);
#endif
        if (recv_ln <= 0
#ifdef _NODE_LINK_
#else
            || task->m_offset == task->m_buff_len
#endif
            )
        {
#ifdef _NODE_LINK_
#else
            task->m_offset -= recv_ln;
#endif
            task->SetSocketClosed(true);
            m_socket->close();
            PopTask();              // delete the last task;
            break;
        }
    }
    return;
}

void SocketInfo::StartSendTask()
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

            LOG_PRINT_WARN("StartSendTask: %s", GetLastTask().c_str());
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
            LOG_PRINT_WARN("WARN: StartSendTask, get: %s, now: %s",
                            session_tmp.c_str(), session_now.c_str());
            continue;
        }
        tmp = NULL;
    }
    return;
}

void SocketInfo::ThreadDetach(std::string command)
{
    // send task ... 
	LOG_PRINT_WARN("Send task start ...");
#if 1
    system(command.c_str());
#else
    m_Audio2Pcm.CallCodecFunction(command);
#endif
    LOG_PRINT_WARN("Send task end ...");
}

std::string SocketInfo::GetTask()
{
    // 等待任务
    while (IsEmpty())
    {
        continue;
    }
    return FrontTask();
}

bool SocketInfo::IsEmpty()
{
    std::unique_lock<std::mutex> wait_lock(m_wait_mutex);
    m_wait_cv.wait(wait_lock); // wait for next task;

    std::lock_guard<std::mutex> lock(m_task_mutex);
    return m_task_sessionId.empty();
}

std::string SocketInfo::FrontTask()
{
    std::lock_guard<std::mutex> lock(m_task_mutex);
    return m_task_sessionId.front();
}

void SocketInfo::PopTask()
{
    std::lock_guard<std::mutex> lock(m_task_mutex);
    LOG_PRINT_WARN("tasknum:%lu", m_task_sessionId.size());
    m_task_sessionId.pop();
    m_wait_cv.notify_one(); // notify handle next task;
    return;
}

void SocketInfo::InsertTask(std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_task_mutex);
    m_task_sessionId.push(sessionId);
    LOG_PRINT_WARN("tasknum:%lu", m_task_sessionId.size());
}

void SocketInfo::SetLastTask(std::string sessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessionId = sessionId;
    return;
}

std::string SocketInfo::GetLastTask()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sessionId;
}


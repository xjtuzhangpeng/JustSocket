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
    TaskInfo *tmp = new TaskInfo(command
#ifdef _NODE_LINK_
#else
                                 , m_buf_len
#endif
                                 );
    m_task_map.insert(std::make_pair(sessionId, tmp));

    InsertTask(sessionId);
    return;
}

void SocketInfo::StartServer(short port)
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

bool SocketInfo::StartSendTask()
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

void SocketInfo::ReceiveData()
{
    int          recv_ln   = 0;
    char        *buf       = NULL;
    size_t       buf_len   = 0;
    std::string  last_task = GetLastTask();

    if (last_task.empty())
    {
        printf ("Error: task: %s \n", last_task.c_str());
        return;
    }

    TaskInfo *task = m_task_map[last_task];
    if (m_socket == NULL || task == NULL)
    {
        printf ("Error: m_socket, task: %p , %p, %s \n", m_socket, task, last_task.c_str());
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
        //printf("m_offset %lu, recv_ln %d \n", node->offset, recv_ln);
#else
        task->m_offset += recv_ln;
        //printf("m_offset %lu, recv_ln %d \n", task->m_offset, recv_ln);
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
            PopTask(); // delete the last task;
            break;
        }
    }
    return;
}

std::string SocketInfo::GetTask()
{
    // 等待任务
    while (IsEmpty())
    {
        usleep(10);
        continue;
    }
    return FrontTask();
}

bool SocketInfo::IsEmpty()
{
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
    printf("Task %s done. \n", m_task_sessionId.front().c_str());
    SetLastTask("");
    m_task_sessionId.pop();
}

void SocketInfo::InsertTask(std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_task_mutex);
    m_task_sessionId.push(sessionId);
}

void SocketInfo::ThreadDetach(std::string command)
{
    // send task ... 
    system(command.c_str());
}


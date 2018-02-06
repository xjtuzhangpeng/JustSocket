#include "Tit_SocketInfo.h"

SocketInfo::SocketInfo(short port
#ifdef _NODE_LINK_
#else
                       , size_t buf_len
#endif
                       ) : 
#ifdef _NODE_LINK_
#else
                       m_buf_len(buf_len), 
#endif
                       m_thread_num(0)
{
    std::thread thrd_server(std::bind(&SocketInfo::StartServer, this, port));
    thrd_server.detach();
    
    std::thread thrd_sender(std::bind(&SocketInfo::StartSendTask, this));
    thrd_sender.detach();
}

SocketInfo::~SocketInfo()
{
}

size_t SocketInfo::GetFFmpegBuffLen(std::string &sessionId)
{
    size_t    len  = -1;

    if (m_task_map.find(sessionId) == m_task_map.end())
    {
        LOG_PRINT_ERROR("sessionId:%s is not in map", sessionId.c_str());
        return len;
    }

    TaskInfo *task = m_task_map[sessionId];
    if (task == NULL)
    {
        LOG_PRINT_ERROR("Task is null");
        return 0; // 任务不存在
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

size_t SocketInfo::GetFFmpegBuff(std::string sessionId, char *buf, size_t buff_len)
{
    size_t    len  = 0;

    if (m_task_map.find(sessionId) == m_task_map.end())
    {
        return len;
    }

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
    m_taskbgn_cv.notify_one(); // wait for next task;
    return;
}

void SocketInfo::StartServer(short port)
{
    CPPTcpServerSocket server_tcp;
    server_tcp.listen(port, MAX_NUM_OF_CONNECT);

    while (true)
    {
        int fd = server_tcp.accept(-1);
        std::thread thrd(std::bind(&SocketInfo::ReceiveData, this, fd, GetLastTask()));
        thrd.detach();
        m_socket_cv.notify_one(); // 同步: 开始转码与socket连接成功
    }
}

void SocketInfo::ReceiveData(int fd, std::string last_task)
{
    int          recv_ln   = 0;
    char        *buf       = NULL;
    size_t       buf_len   = 0;
    TaskInfo    *task      = m_task_map[last_task];
    CPPSocket    socket(fd);

    //LOG_PRINT_INFO("Task: %p, %s", task, last_task.c_str());

    if (last_task.empty() || task == NULL)
    {
        LOG_PRINT_WARN("Error: task: %p, %s", task, last_task.c_str());
        goto RECV_END;
    }

    while (true)
    {
#ifdef _NODE_LINK_
        buf     = task->m_buff_link.GetBuff();
        buf_len = task->m_buff_link.GetTailBuffLen();
#else
        buf     = task->m_buff + task->m_offset;
        buf_len = task->m_buff_len - task->m_offset;
#endif
        socket.hasData(-1);
        recv_ln = socket.recv(buf, buf_len, 0);

        if (recv_ln <= 0
#ifdef _NODE_LINK_
#else
            || task->m_offset == task->m_buff_len
#endif
            )
        {
            task->SetSocketClosed(true);
            break;
        }
#ifdef _NODE_LINK_
        task->m_buff_link.UpdateBuffLen(recv_ln);
        //LOG_PRINT_INFO("Task: %s, m_offset %lu, recv_ln %d", last_task.c_str(), task->m_buff_link.GetTail()->offset, recv_ln);
#else
        task->m_offset += recv_ln;
        //LOG_PRINT_INFO("Task: %s, m_offset %lu, recv_ln %d", last_task.c_str(), task->m_offset, recv_ln);
#endif
    }

RECV_END:
    socket.close();
    MinusWorkThread();
    m_taskend_cv.notify_one();
    return;
}

void SocketInfo::StartSendTask()
{
    TaskInfo *tmp = NULL;
    while (true)
    {
        if (GetWorkThreadNum() < MAX_NUM_OF_THREAD)
        {
            std::string session_tmp = GetTask();

            tmp = m_task_map[session_tmp];
            if (session_tmp.empty() || tmp == NULL)
            {
                continue;
            }

            PlusWorkThread();
            SetLastTask(session_tmp);
            LOG_PRINT_INFO("StartSendTask: %s, num %d", GetLastTask().c_str(), GetWorkThreadNum());
            std::thread thrd(std::bind(&SocketInfo::ThreadDetach, this, tmp->m_command));
            thrd.detach();

            std::unique_lock<std::mutex> wait_lock(m_socket_mutex);
            m_socket_cv.wait(wait_lock); // wait for socket allready task;
        }
        else
        {
            LOG_PRINT_WARN("too many tasks %d, wait for notify ..", GetWorkThreadNum());
            std::unique_lock<std::mutex> wait_lock(m_taskend_mutex);
            m_taskend_cv.wait(wait_lock); // wait for task finish;
            continue;
        }
    }
    return;
}

void SocketInfo::ThreadDetach(std::string command)
{
    // send task ... 
	LOG_PRINT_INFO("Send task start ...");
#if 1
    system(command.c_str());
#else
    m_Audio2Pcm.CallCodecFunction(command);
#endif
    LOG_PRINT_INFO("Send task end ...");
}

std::string SocketInfo::GetTask()
{
    // 等待任务
    if (IsEmpty())
    {
        std::unique_lock<std::mutex> wait_lock(m_taskbgn_mutex);
        m_taskbgn_cv.wait(wait_lock); // wait for a new task;
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
    std::string sid = m_task_sessionId.front();
    m_task_sessionId.pop();
    return sid;
}

void SocketInfo::InsertTask(std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_task_mutex);
    m_task_sessionId.push(sessionId);
    LOG_PRINT_INFO("tasknum:%lu", m_task_sessionId.size());
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

void SocketInfo::PlusWorkThread()
{
    std::lock_guard<std::mutex> lock(m_thread_mutex);
    m_thread_num++;
    return;
}

void SocketInfo::MinusWorkThread()
{
    std::lock_guard<std::mutex> lock(m_thread_mutex);
    if (m_thread_num > 0)
        m_thread_num--;
    else
        LOG_PRINT_ERROR("m_thread_num: %lu", m_thread_num);
    return;
}

size_t SocketInfo::GetWorkThreadNum()
{
    std::lock_guard<std::mutex> lock(m_thread_mutex);
    return m_thread_num;
}



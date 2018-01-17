#include <thread>
#include <stdio.h>
#include "ServerMain.h"

#define MAX_BUFF_LEN       (10 * 1024 * 1024)

class SocketInfo
{
public:
    SocketInfo(int fd)
	{
		offset       = 0;
        m_socket_ptr = new CPPSocket(fd);
		m_buf        = new char[MAX_BUFF_LEN];
	}

	~SocketInfo()
	{
	    delete m_socket_ptr;
		delete m_buf;
	}

	void ReceiveData()
	{
	    int recv_ln = 0;
	    while (true)
    	{
		    m_socket_ptr->hasData(-1);
			recv_ln = m_socket_ptr->recv(m_buf + offset, MAX_BUFF_LEN - offset, 0);

			offset += recv_ln;
            printf("offset %lu, recv_ln %d \n", offset, recv_ln);
			if (0 == recv_ln)
			{
				m_socket_ptr->close();
				break;
			}
    	}
		return;
	}

    size_t GetBuff(char **buf_ptr_ptr)
	{
	    *buf_ptr_ptr = m_buf;
		return offset;
	}
private:
	CPPSocket *m_socket_ptr;
    char      *m_buf;
	size_t     offset;
};

void Handler(int fd)
{
	SocketInfo *info = new SocketInfo(fd);
	info->ReceiveData();
	delete info;
}

int main(int agrs, char *argv[])
{
    CPPTcpServerSocket server;
	server.listen(18080, 1);

    while (true)
	{
		int fd = server.accept(-1);
		std::thread thrd(std::bind(Handler, fd));
		thrd.detach();
	}
	
    return 0;
}


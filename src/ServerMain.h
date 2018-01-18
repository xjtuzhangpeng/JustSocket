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

#define _NODE_LINK_
#ifdef  _NODE_LINK_
#define MAX_BUFF_LEN       (1 * 1024 * 1024)
#define BUFF_NODE_LEN      (1 * 1024 * 1024)
#else
#define MAX_BUFF_LEN       (100 * 1024 * 1024)
#endif
#include "Tit_Map.h"
#include "Tit_BiffLink.h"
#include "Tit_TaskInfo.h"
#include "Tit_SocketInfo.h"

static inline std::string int2str(const int &input)
{
    std::string        output;
    std::ostringstream oStream;

    oStream << input;
    output = oStream.str();

    return output;
}

#endif//_SERVER_MAIN_H_

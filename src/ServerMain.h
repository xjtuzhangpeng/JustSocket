#ifndef _SERVER_MAIN_H_
#define _SERVER_MAIN_H_
#include <iostream>
#include <sstream>

#include "Tit_BuffLink.h"
#include "Tit_Map.h"
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

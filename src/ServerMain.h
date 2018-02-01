#ifndef _SERVER_MAIN_H_
#define _SERVER_MAIN_H_
#include <iostream>
#include <sstream>
#include <new>

#include "common.h"
#include "Tit_BuffLink.h"
#include "Tit_Map.h"
#include "Tit_TaskInfo.h"
#include "Tit_SocketInfo.h"
#include "Tit_Resample.h"
#include "Tit_MonoStereo.h"
#include "Tit_AVIOReading.h"

#define SOX_VOX_1(tempWavName, outWavName) \
    "sox -e oki-adpcm -b 4 -r 6k " + tempWavName + " -b 16 -r 8000 " + outWavName + " highpass 10"

static inline std::string int2str(const int &input)
{
    std::string        output;
    std::ostringstream oStream;

    oStream << input;
    output = oStream.str();

    return output;
}


#endif//_SERVER_MAIN_H_

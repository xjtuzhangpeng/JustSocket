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

static char PCM_HEAD[44] = 
{
    0x52, 0x49, 0x46, 0x46, 0xe4, 0x2b, 0x3c, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20, 
    0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00, 
    0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0xc0, 0x2b, 0x3c, 0x00
};

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

#ifndef _TIT_LOGGER_H_
#define _TIT_LOGGER_H_
#include <memory.h>

#include "log4cplus/loggingmacros.h"
#include "log4cplus/logger.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/layout.h"
#include "log4cplus/ndc.h"
#include "log4cplus/helpers/loglog.h"

#define TIT_MAX_LOG_BUFF     1024

enum eLogLevel
{
    TIT_LOG_TRACE,
    TIT_LOG_DEBUG,
    TIT_LOG_INFO,
    TIT_LOG_WARN,
    TIT_LOG_ERROR,
    TIT_LOG_FATAL
};

class LOGGER
{
public:
    LOGGER(std::string filename, int logLevel);
    ~LOGGER();

    void LogPrint(enum eLogLevel level, const char *txt, ...);
    int  GetLogLevel();
private:
    log4cplus::Logger    m_pTestLogger;
    int                  m_logLevel;
};

extern LOGGER G_Log;

#if 1
#define  LOG_PRINT_TRACE(fmt, ...)   \
    do { G_Log.LogPrint(TIT_LOG_TRACE, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_DEBUG(fmt, ...)   \
    do { G_Log.LogPrint(TIT_LOG_DEBUG, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_INFO(fmt, ...)    \
    do { G_Log.LogPrint(TIT_LOG_INFO,  fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_WARN(fmt, ...)    \
    do { G_Log.LogPrint(TIT_LOG_WARN,  fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_ERROR(fmt, ...)   \
    do { G_Log.LogPrint(TIT_LOG_ERROR, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_FATAL(fmt, ...)   \
    do { G_Log.LogPrint(TIT_LOG_FATAL, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#else
#define  LOG_PRINT_TRACE(fmt, ...)   printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_DEBUG(fmt, ...)   printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_INFO(fmt, ...)    printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_WARN(fmt, ...)    printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_ERROR(fmt, ...)   printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_FATAL(fmt, ...)   printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#endif

#endif//_TIT_LOGGER_H_

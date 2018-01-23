#ifndef _TIT_LOGGER_H_
#define _TIT_LOGGER_H_
#include <memory>
#include <mutex>

#include "log4cplus/loggingmacros.h"
#include "log4cplus/logger.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/layout.h"
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

#define TIT_MAX_LOG_BUFF     1024

enum eLogLevel
{
    TIT_LOG_DEBUG,
    TIT_LOG_INFO,
    TIT_LOG_WARN,
    TIT_LOG_ERROR,
};

class LOGGER
{
public:
    LOGGER(std::string filename)
    {
        //std::string filename = "./2_buff_ffmpeg.log";
        std::string pattern  = "%D{%m/%d/%y %H:%M:%S} - [%p] %m%n";
        // 定义1个控制台的Appender, 1个文件Appender
        // SharedAppenderPtr pConsoleAppender(new ConsoleAppender());
        SharedAppenderPtr pFileAppender(new FileAppender(tstring(filename)));
        
        // 定义一个PatternLayout,并绑定到 pFileAppender
        std::auto_ptr<Layout> pPatternLayout(new PatternLayout(tstring(pattern)));
        pFileAppender->setName(LOG4CPLUS_TEXT("test"));
        pFileAppender->setLayout(pPatternLayout);
        
        // 定义Logger
        m_pTestLogger = Logger::getInstance(tstring(filename));
        
        // 将需要关联Logger的Appender添加到Logger上
        m_pTestLogger.addAppender(pFileAppender);
        
        // 输出日志信息
        LOG4CPLUS_WARN(m_pTestLogger, "This is a <Warn> log message...");
        m_buffer = new char[TIT_MAX_LOG_BUFF];
    }

    ~LOGGER()
    {
    }

    void LogPrint(enum eLogLevel level, const char *txt, ...)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        va_list marker;	
    	memset  (m_buffer, 0x00, TIT_MAX_LOG_BUFF);
    	va_start(marker, txt);
    	vsprintf(m_buffer, txt, marker);
    	va_end  (marker);

        switch(level)
        {
            case TIT_LOG_DEBUG:
                LOG4CPLUS_DEBUG(m_pTestLogger, m_buffer);
                break;
            case TIT_LOG_INFO:
                LOG4CPLUS_INFO(m_pTestLogger,  m_buffer);
                break;
            case TIT_LOG_WARN:
                LOG4CPLUS_WARN(m_pTestLogger,  m_buffer);
                break;
            case TIT_LOG_ERROR:
                LOG4CPLUS_ERROR(m_pTestLogger, m_buffer);
                break;
            default :
                LOG4CPLUS_TRACE(m_pTestLogger, m_buffer);
                break;
        }
    }
private:
    Logger       m_pTestLogger;
    std::mutex   m_mutex;
    char        *m_buffer;

};

extern LOGGER G_Log;

#if 1
#define  LOG_PRINT_DEBUG(fmt, ...)   \
    do { G_Log.LogPrint(TIT_LOG_DEBUG, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_INFO(fmt, ...)    \
    do { G_Log.LogPrint(TIT_LOG_INFO,  fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_WARN(fmt, ...)    \
    do { G_Log.LogPrint(TIT_LOG_WARN,  fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_ERROR(fmt, ...)   \
    do { G_Log.LogPrint(TIT_LOG_ERROR, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#else
#if 0
#define  LOG_PRINT_DEBUG(fmt, ...)   printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_INFO(fmt, ...)    printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_WARN(fmt, ...)    printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#define  LOG_PRINT_ERROR(fmt, ...)   printf(fmt"(%s:%d)\n", ##__VA_ARGS__,__FILE__,__LINE__)
#else
#define  LOG_PRINT_DEBUG(fmt, ...)   
#define  LOG_PRINT_INFO(fmt, ...)    
#define  LOG_PRINT_WARN(fmt, ...)    
#define  LOG_PRINT_ERROR(fmt, ...)   
#endif
#endif

#endif//_TIT_LOGGER_H_

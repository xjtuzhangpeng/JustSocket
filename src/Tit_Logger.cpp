#include "Tit_Logger.h"

using namespace log4cplus;
using namespace log4cplus::helpers;

LOGGER::LOGGER(std::string filename, int logLevel)
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
    m_logLevel = logLevel;
}

LOGGER::~LOGGER()
{
}

int  LOGGER::GetLogLevel()
{
    return m_logLevel;
}

void LOGGER::LogPrint(enum eLogLevel level, const char *txt, ...)
{
    if (level < m_logLevel)
    {
        return;
    }
    char *buffer = new char[TIT_MAX_LOG_BUFF];
    va_list marker;	
	memset  (buffer, 0x00, TIT_MAX_LOG_BUFF);
	va_start(marker, txt);
	vsprintf(buffer, txt, marker);
	va_end  (marker);

    switch(level)
    {
        case TIT_LOG_TRACE:
            LOG4CPLUS_TRACE(m_pTestLogger, buffer);
            break;
        case TIT_LOG_DEBUG:
            LOG4CPLUS_DEBUG(m_pTestLogger, buffer);
            break;
        case TIT_LOG_INFO:
            LOG4CPLUS_INFO(m_pTestLogger,  buffer);
            break;
        case TIT_LOG_WARN:
            LOG4CPLUS_WARN(m_pTestLogger,  buffer);
            break;
        case TIT_LOG_ERROR:
            LOG4CPLUS_ERROR(m_pTestLogger, buffer);
            break;
        case TIT_LOG_FATAL:
            LOG4CPLUS_FATAL(m_pTestLogger, buffer);
            break;
        default :
            LOG4CPLUS_TRACE(m_pTestLogger, buffer);
            break;
    }
    delete buffer;
    buffer = NULL;
}

extern "C" void LOG_PRINT_C(enum eLogLevel level, char * line, ...)
{
    if (level < G_Log.GetLogLevel())
    {
        return;
    }
    
    char *buffer = new char[TIT_MAX_LOG_BUFF];
    va_list marker; 
    memset  (buffer, 0x00, TIT_MAX_LOG_BUFF);
    va_start(marker, line);
    vsprintf(buffer, line, marker);
    va_end  (marker);

    G_Log.LogPrint(level, "%s", buffer);

    delete buffer;
    buffer = NULL;
}


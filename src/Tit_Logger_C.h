#ifndef _TIT_LOGGER_C_H_
#define _TIT_LOGGER_C_H_

enum eLogLevel
{
    TIT_LOG_TRACE_C,
    TIT_LOG_DEBUG_C,
    TIT_LOG_INFO_C,
    TIT_LOG_WARN_C,
    TIT_LOG_ERROR_C,
    TIT_LOG_FATAL_C
};

// C语言日志接口
extern void LOG_PRINT_C(enum eLogLevel level, char * line, ...);
#define  LOG_PRINT_TRACE_C(fmt, ...)   \
        do { LOG_PRINT_C(TIT_LOG_TRACE_C, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_DEBUG_C(fmt, ...)   \
        do { LOG_PRINT_C(TIT_LOG_DEBUG_C, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_INFO_C(fmt, ...)    \
        do { LOG_PRINT_C(TIT_LOG_INFO_C,  fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_WARN_C(fmt, ...)    \
        do { LOG_PRINT_C(TIT_LOG_WARN_C,  fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_ERROR_C(fmt, ...)   \
        do { LOG_PRINT_C(TIT_LOG_ERROR_C, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#define  LOG_PRINT_FATAL_C(fmt, ...)   \
        do { LOG_PRINT_C(TIT_LOG_FATAL_C, fmt" (%s:%d)", ##__VA_ARGS__, __FILE__, __LINE__); } while(0)
#endif//_TIT_LOGGER_C_H_


/********************************************************************************
 * The functions below work with SOX
 ********************************************************************************/
#include "Tit_SoxBuf.h"
#include "Tit_Logger.h"
#include "Tit_Map.h"

static TIT_Map<std::string, TIT_FILE *> g_SoxBufMap;
static TIT_Map<std::string, TIT_FILE *> g_SoxFileMap;

#ifdef __cplusplus
extern "C" {
#endif
    size_t ReadTitBuff(const char * filename, void *buf, size_t len)
    {
        if (filename == NULL || buf == NULL)
        {
            LOG_PRINT_WARN("filename %p, buf %p", filename, buf);
            return 0;
        }

        std::string fileName = filename;
        if (len == 0 || g_SoxFileMap.find(fileName) == g_SoxFileMap.end())
        {
            LOG_PRINT_WARN("len %lu, filename: %s", len, filename);
            return 0;
        }

        TIT_FILE * file = g_SoxFileMap[fileName];
        size_t     tmp  = len > (file->buf_len - file->offset) ? (file->buf_len - file->offset) : len;
        memcpy(buf, file->buf + file->offset, tmp);
        file->offset += tmp;

        //if (file->offset == file->buf_len)
        {
            //g_SoxFileMap.erase(g_SoxFileMap.find(fileName));
        }
        return tmp;
    }

    int TitSeek(const char * filename, long long offset, int whence)
    {
        std::string fileName = filename;
        if (g_SoxFileMap.find(fileName) != g_SoxFileMap.end())
        {
            InBufSeek(fileName, offset, whence);
        }
        else if (g_SoxBufMap.find(fileName) != g_SoxBufMap.end())
        {
            OutBufSeek(fileName, offset, whence);
        }
        else
        {
            LOG_PRINT_ERROR("%s", filename);
        }
        
        return -1;
    }

    size_t WriteTitBuff(const char *filename, const char *buf, size_t len)
    {
        std::string fileName = filename;

        if (g_SoxBufMap.find(fileName) == g_SoxBufMap.end())
        {
            LOG_PRINT_TRACE("new a Bufflink");
            TIT_FILE * file = new TIT_FILE();
            file->buf       = new char[100 * 1024 * 1024];
            file->buf_len   = 100 * 1024 * 1024;
            file->offset    = 0;
            g_SoxBufMap[fileName] = file;
        }

        TIT_FILE *file = g_SoxBufMap[fileName];

        memcpy((file->buf + file->offset), buf, len);
        file->offset += len;
        //LOG_PRINT_INFO("%s ----- %lu", filename, len);

        return len;
    }
#ifdef __cplusplus
}
#endif

int  InBufSeek(std::string filename, long long offset, int whence)
{
    TIT_FILE * file = g_SoxFileMap[filename];
    
    switch (whence)
    {
    case SEEK_SET:
        file->offset  = offset;
        break;
    case SEEK_CUR:
        file->offset += offset;
        break;
    case SEEK_END:
        file->offset  = file->buf_len + offset;
        break;
    default:
        return -1;
    }
    LOG_PRINT_DEBUG("offset %lu, buf_len %lu, file_offset %lu, whence %d; filename: %s ", 
                    offset, file->buf_len, file->offset, whence, filename.c_str());
    if (file->offset >= file->buf_len)
        return -1;
    return 0;
}

int OutBufSeek(std::string filename, long long offset, int whence)
{
    TIT_FILE *file        = g_SoxBufMap[filename];

    switch (whence)
    {
    case SEEK_SET:
        file->offset  = offset;
        break;
    case SEEK_CUR:
        file->offset += offset;
        break;
    case SEEK_END:
        file->offset  = file->buf_len + offset;
        break;
    default:
        return -1;
    }

    LOG_PRINT_DEBUG("offset %lu, buf_len %lu, file_offset %lu, whence %d; filename: %s ", 
                    offset, file->buf_len, file->offset, whence, filename.c_str());
    if (file->offset >= file->buf_len)
        return -1;
    return 0;
}

bool StoreTitBuff(std::string filename, char *buf, size_t len)
{
    if (filename.empty() || len == 0)
    {
        LOG_PRINT_ERROR("filename:%s, len%lu", filename.c_str(), len);
        return false;
    }
    LOG_PRINT_DEBUG("filename:%s, len%lu", filename.c_str(), len);

    if (g_SoxFileMap.find(filename) != g_SoxFileMap.end())
    {
        TIT_FILE * file = g_SoxFileMap[filename];
        //delete file->buf; // no need delete, delete by Audio2pcm;
        //file->buf = NULL;
        delete file;
		file = NULL;
        g_SoxFileMap.erase(g_SoxFileMap.find(filename));
    }

    TIT_FILE * file = new TIT_FILE();
    file->buf       = buf;
    file->buf_len   = len;
    file->offset    = 0;
    g_SoxFileMap[filename] = file;
    return true;
}

size_t GetSoxBufLen(std::string filename)
{
    if (g_SoxBufMap.find(filename) == g_SoxBufMap.end())
    {
        return 0;
    }
    TIT_FILE *link = g_SoxBufMap[filename];

    return link->offset;
}

size_t CopySoxBuf(std::string filename, char *buff, size_t len)
{
    if (g_SoxBufMap.find(filename) == g_SoxBufMap.end())
    {
        return 0;
    }
    TIT_FILE *link    = g_SoxBufMap[filename];
    memcpy(buff, link->buf, len);

    g_SoxBufMap.erase(g_SoxBufMap.find(filename));

    delete link->buf;
    link->buf = NULL;
    delete link;
    link = NULL;
    return len;
}
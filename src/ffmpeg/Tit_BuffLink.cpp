#include "Tit_BuffLink.h"

#ifdef  _NODE_LINK_
BuffLink::BuffLink()
{
    tail = head = NULL;
}

BuffLink::~BuffLink()
{
    BuffNode *tmp       = head;
    BuffNode *to_delate = NULL;
    for (;;)
    {
        if (tmp == NULL)
        {
            break;
        }
        to_delate = tmp;
        tmp       = tmp->next;

        //LOG_PRINT_INFO("delete one buff to_delate:%p", to_delate);

        delete to_delate;
        to_delate = NULL;
    }
}

void BuffLink::Add()
{
    BuffNode *tmp = new BuffNode();
    //LOG_PRINT_INFO("new one buff link:%p", tmp);
    if (tail == NULL)
    {
        tail = head = tmp;
    }
    else
    {
        tail->next = tmp;
        tail       = tmp;
    }
}

void BuffLink::Add(size_t buf_len)
{
    BuffNode *tmp = new BuffNode(buf_len);
    //LOG_PRINT_INFO("new one buff link:%p", tmp);
    if (tail == NULL)
    {
        tail = head = tmp;
    }
    else
    {
        tail->next = tmp;
        tail       = tmp;
    }
}

BuffNode * BuffLink::GetTail()
{
    if (NULL == tail || BUFF_NODE_LEN == tail->offset)
    {
        Add();
    }
    return tail;
}


BuffNode * BuffLink::GetTail(size_t buf_len)
{
    Add(buf_len);
    return tail;
}

char * BuffLink::GetBuff()
{
    (void)GetTail();
    return tail->buff + tail->offset;
}

size_t BuffLink::GetTailBuffLen()
{
    (void)GetTail();
    return (BUFF_NODE_LEN - tail->offset);
}

void BuffLink::UpdateBuffLen(size_t diff)
{
    if (diff > (BUFF_NODE_LEN - tail->offset))
    {
        //LOG_PRINT_WARN("the input size %lu is too long -- ", diff);
        diff = BUFF_NODE_LEN - tail->offset;
        //LOG_PRINT_WARN(" -- cut it to %d", diff);
    }
    tail->offset += diff;
    return;
}

size_t BuffLink::BuffLen()
{
    size_t           len = 0;
    BuffNode        *tmp = head;
    for (;;)
    {
        if (tmp == NULL)
        {
            break;
        }

        len += tmp->offset;
        tmp  = tmp->next;
    }

    return len;
}

size_t BuffLink::CopyBuff(char *buf, size_t buf_len)
{
    size_t           len     = 0;
    size_t           cpy_len = 0;
    BuffNode        *tmp     = head;
    for (;;)
    {
        if (tmp == NULL)
        {
            break;
        }

        if ((len + tmp->offset) > buf_len)
        {
            cpy_len = buf_len - len;
        }
        else
        {
            cpy_len = tmp->offset;
        }
        memcpy(buf + len, tmp->buff, cpy_len);
        len += cpy_len;
        tmp  = tmp->next;
    }

    return len;
}

/********************************************************************************
 * The functions below work with SOX
 ********************************************************************************/
#include "Tit_Map.h"
typedef struct TIT_FILE
{
    char     *buf;
    size_t    buf_len;
    size_t    offset;
}TIT_FILE;
static TIT_Map<std::string, BuffLink *> g_SoxBufMap;
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
            //LOG_PRINT_WARN("len %lu", len);
            return 0;
        }

        TIT_FILE * file = g_SoxFileMap[fileName];
        size_t     tmp  = len > (file->buf_len - file->offset) ? (file->buf_len - file->offset) : len;
        memcpy(buf, file->buf + file->offset, tmp);
        file->offset += tmp;

        if (file->offset == file->buf_len)
        {
            g_SoxFileMap.erase(g_SoxFileMap.find(fileName));
        }
        return tmp;
    }

    size_t WriteTitBuff(const char *filename, const char *buf, size_t len)
    {
        std::string fileName = filename;

        if (g_SoxBufMap.find(fileName) == g_SoxBufMap.end())
        {
            LOG_PRINT_TRACE("new a Bufflink");
            g_SoxBufMap[fileName] = new BuffLink();
        }

        BuffLink *link = g_SoxBufMap[fileName];
        BuffNode *node = link->GetTail(len);

        memcpy(node->buff, buf, len);
        node->offset = len;
        //LOG_PRINT_INFO("%s ----- %lu", filename, len);

        return len;
    }
#ifdef __cplusplus
}
#endif

bool StoreTitBuff(std::string filename, char *buf, size_t len)
{
    if (filename.empty() || len == 0)
    {
        LOG_PRINT_ERROR("filename: %s, len %lu", filename.c_str(), len);
        return false;
    }

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
    BuffLink *link = g_SoxBufMap[filename];

    return link->BuffLen();
}

size_t CopySoxBuf(std::string filename, char *buff, size_t len)
{
    if (g_SoxBufMap.find(filename) == g_SoxBufMap.end())
    {
        return 0;
    }
    BuffLink *link    = g_SoxBufMap[filename];
    size_t    tmp_len = link->CopyBuff(buff, len);

    g_SoxBufMap.erase(g_SoxBufMap.find(filename));
    delete link;
    link = NULL;
    return tmp_len;
}
#endif

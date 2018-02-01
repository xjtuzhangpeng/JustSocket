#include "Tit_BuffLink.h"
#include "Tit_Map.h"

static TIT_Map<BuffLink *> g_SoxBufMap;

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

extern "C" void * GetTitBuff(const char *filename, size_t buff_len, size_t sample_len)
{
    std::string fileName = filename;

    if (g_SoxBufMap.find(fileName) != g_SoxBufMap.end())
    {
        g_SoxBufMap[fileName] = new BuffLink();
    }

    BuffLink *link = g_SoxBufMap[fileName];
    BuffNode *node = link->GetTail(buff_len);

    LOG_PRINT_WARN(" ----- ");

    return (void *)node->buff;
}

size_t GetSoxBufLen(std::string filename)
{
    if (g_SoxBufMap.find(filename) != g_SoxBufMap.end())
    {
        return 0;
    }
    BuffLink *link = g_SoxBufMap[filename];

    return link->BuffLen();
}

size_t CopySoxBuf(std::string filename, char *buff, size_t len)
{
    if (g_SoxBufMap.find(filename) != g_SoxBufMap.end())
    {
        return 0;
    }
    BuffLink *link = g_SoxBufMap[filename];
    return link->CopyBuff(buff, len);
}
#endif

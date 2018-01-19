#include "Tit_BuffLink.h"

#ifdef  _NODE_LINK_
BuffLink::BuffLink()
{
    link = new BuffNode();
    tail = head = link;
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

        delete to_delate;
        to_delate = NULL;
    }
}

void BuffLink::Add()
{
    BuffNode *tmp = new BuffNode();
    tail->next = tmp;
    tail       = tmp;
}

BuffNode * BuffLink::GetTail()
{
    if (tail->offset == BUFF_NODE_LEN)
    {
        Add();
    }
    return tail;
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
#endif

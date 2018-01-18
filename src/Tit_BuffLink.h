#ifndef _TIT_BUFF_LINK_H_
#define _TIT_BUFF_LINK_H_

struct BuffNode
{
    size_t            offset;
    char             *buff;
    struct BuffNode  *next;

    BuffNode() : offset(0), next(NULL)
    {
        buff = new char[BUFF_NODE_LEN];
    }
};

class BuffLink
{
public:
    BuffLink()
    {
        link = new BuffNode();
        tail = head = link;
    }

    ~BuffLink()
    {
        struct BuffNode *tmp       = head;
        struct BuffNode *to_delate = NULL;
        for (;;)
        {
            if (tmp == NULL)
            {
                break;
            }
            to_delate = tmp;
            tmp       = tmp->next;

            delete to_delate->buff;
            to_delate->buff = NULL;
            delete to_delate;
            to_delate = NULL;
        }
    }

    struct BuffNode * GetTail()
    {
        if (tail->offset == BUFF_NODE_LEN)
        {
            Add();
        }
        return tail;
    }

    size_t BuffLen()
    {
        size_t           len = 0;
        struct BuffNode *tmp = head;
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

    size_t CopyBuff(char *buf, size_t buf_len)
    {
        size_t           len     = 0;
        size_t           cpy_len = 0;
        struct BuffNode *tmp     = head;
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

private:
    void Add()
    {
        struct BuffNode *tmp = new BuffNode();
        tail->next = tmp;
        tail       = tmp;
    }

    struct BuffNode *link;
    struct BuffNode *head;
    struct BuffNode *tail;
};

#endif//_TIT_BUFF_LINK_H_
#ifndef _TIT_BUFF_LINK_H_
#define _TIT_BUFF_LINK_H_

#define _NODE_LINK_

class BuffNode
{
public:
    size_t            offset;
    char             *buff;
    BuffNode         *next;

    BuffNode() : offset(0), next(NULL)
    {
        buff = new char[BUFF_NODE_LEN];
    }

    ~BuffNode()
    {
        delete buff;
        buff = NULL;
    }
};

class BuffLink
{
public:
    BuffLink();
    ~BuffLink();

    size_t BuffLen();
    size_t CopyBuff(char *buf, size_t buf_len);

    BuffNode * GetTail();
private:
    void Add();

    BuffNode     *link;
    BuffNode     *head;
    BuffNode     *tail;
};

#endif//_TIT_BUFF_LINK_H_

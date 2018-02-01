#ifndef _TIT_BUFF_LINK_H_
#define _TIT_BUFF_LINK_H_
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "Tit_Logger.h"

#define _NODE_LINK_

#ifdef  _NODE_LINK_
#define BUFF_NODE_LEN      (1 * 1024 * 1024)

class BuffNode
{
public:
    size_t            offset;
    size_t            buff_len;
    char             *buff;
    BuffNode         *next;

    BuffNode() : offset(0), next(NULL)
    {
        buff = new char[BUFF_NODE_LEN];
        //LOG_PRINT_INFO("new one buff buff:%p", buff);
    }

    BuffNode(size_t buf_len) : offset(buf_len), next(NULL)
    {
        buff = new char[buf_len];
        //LOG_PRINT_INFO("new one buff buff:%p", buff);
    }

    ~BuffNode()
    {
        delete buff;
        //LOG_PRINT_INFO("delete one buff offset:%d, buff:%p", offset, buff);
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

    char  *GetBuff();
    size_t GetTailBuffLen();
    void   UpdateBuffLen(size_t diff);
    
    BuffNode * GetTail();
    BuffNode * GetTail(size_t buf_len);
private:
    void Add();
    void Add(size_t buf_len);

    BuffNode     *head;
    BuffNode     *tail;
};
#endif

#endif//_TIT_BUFF_LINK_H_

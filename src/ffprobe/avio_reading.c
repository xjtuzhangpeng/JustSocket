/*
 * Copyright (c) 2014 Stefano Sabatini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * libavformat AVIOContext API example.
 *
 * Make libavformat demuxer access media content through a custom
 * AVIOContext read callback.
 * @example avio_reading.c
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>

#include "Tit_Logger_C.h"

struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

#define LINE_SZ 1024
typedef char LINE_BUFF[LINE_SZ];

static int         g_SessionNum = 0;
static LINE_BUFF * g_AudioFormatInfo = NULL;

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);

    //printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

static void InitInfo(int sessionId)
{
    if (sessionId >= 0 && sessionId < g_SessionNum)
    {
        memset(g_AudioFormatInfo[sessionId], 0x00, LINE_SZ);
    }
}

static int PrintfInfo(int sessionId)
{
    LOG_PRINT_DEBUG_C("%s", g_AudioFormatInfo[sessionId]);
    return 0;
}

size_t GetFormatInfo(int sessionId, char *buf, size_t buf_len)
{
    size_t len = 0;
    if (sessionId >= 0 && sessionId < g_SessionNum)
    {
        len = strlen(g_AudioFormatInfo[sessionId]);
        if (buf_len <= len)
        {
            len = buf_len - 1;
        }
        memcpy(buf, g_AudioFormatInfo[sessionId], len);
        buf[len] = '\0';
    }
    return len;
}

size_t GetFormatInfoLen(int sessionId)
{
    size_t len = 0;
    if (sessionId >= 0 && sessionId < g_SessionNum)
    {
        len = strlen(g_AudioFormatInfo[sessionId]);
    }
    return len;
}

void tit_store_log(char * line, int sessionId)
{
    printf("sessionId %d\n", sessionId);
    if (sessionId >= 0 && sessionId < g_SessionNum)
    {
        memcpy(g_AudioFormatInfo[sessionId], line, strlen(line));
    }
}

void InitSessionNum(int sessionNum)
{
    g_SessionNum      = sessionNum;
    if (g_AudioFormatInfo != NULL)
        free(g_AudioFormatInfo);
    g_AudioFormatInfo = malloc((sizeof(LINE_BUFF) * sessionNum));

    printf("%p %p %p \n", g_AudioFormatInfo[0], g_AudioFormatInfo[1], g_AudioFormatInfo[2]);
}

int avio_reading_main(char *filename, int sessionid)
{
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;
    uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    char *input_filename = NULL;
    int ret = 0;
    struct buffer_data bd = { 0 };

    input_filename = filename;
    if (sessionid >= 0 && sessionid < g_SessionNum)
    {
        InitInfo(sessionid);
    }
    else
    {
        return 1;
    }
    

    /* register codecs and formats and other lavf/lavc components*/
    av_register_all();

    /* slurp file content into buffer */
    ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL);
    if (ret < 0)
        goto end;

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr  = buffer;
    bd.size = buffer_size;

    if (!(fmt_ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_packet, NULL, NULL);
    if (!avio_ctx) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    fmt_ctx->pb = avio_ctx;

    ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open input\n");
        goto end;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        goto end;
    }

    av_dump_format(fmt_ctx, sessionid, input_filename, 0); // 获取文件的格式 - zhangpeng
    PrintfInfo(sessionid);

end:
    avformat_close_input(&fmt_ctx);
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }
    av_file_unmap(buffer, buffer_size);

    if (ret < 0) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}


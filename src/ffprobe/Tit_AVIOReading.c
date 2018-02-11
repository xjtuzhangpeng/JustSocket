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

#define STATIC static
#define PUBLIC
#define OFFSET_OF_SESSION 0xAABB // to be a special stream index
#define SESSION_TO_INDEX(sid) ((sid) + OFFSET_OF_SESSION)
#define INDEX_TO_SESSION(idx) ((idx) - OFFSET_OF_SESSION)
#define LINE_SZ 1024
typedef char LINE_BUFF[LINE_SZ];

static int         g_SessionNum = 0;
static LINE_BUFF * g_AudioFormatInfo = NULL;

STATIC int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);

    LOG_PRINT_TRACE_C("ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

STATIC void InitInfo(int sessionId)
{
    if (sessionId >= 0 && sessionId < g_SessionNum)
    {
        memset(g_AudioFormatInfo[(sessionId)], 0x00, LINE_SZ);
    }
}

STATIC int PrintfInfo(int sessionId)
{
    LOG_PRINT_DEBUG_C("%s", g_AudioFormatInfo[(sessionId)]);
    return 0;
}

PUBLIC char * GetFormatInfo(int sessionId)
{
    if (sessionId >= 0 && sessionId < g_SessionNum)
    {
        return g_AudioFormatInfo[sessionId];
    }
    return NULL;
}

PUBLIC size_t GetFormatInfoLen(int sessionId)
{
    size_t len = 0;
    if (sessionId >= 0 && sessionId < g_SessionNum)
    {
        len = strlen(g_AudioFormatInfo[(sessionId)]);
    }
    return len;
}

PUBLIC void InitSessionNum(int sessionNum)
{
    g_SessionNum = sessionNum;
    if (g_AudioFormatInfo != NULL)
        free(g_AudioFormatInfo);
    g_AudioFormatInfo = malloc((sizeof(LINE_BUFF) * sessionNum));
}

PUBLIC int AVIOReading(char *filename, int sessionid)
{
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;
    uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
    size_t buffer_size, avio_ctx_buffer_size = 4096;
    char *input_filename = NULL;
    int ret = 0;
    struct buffer_data bd = { 0 };

    input_filename = filename;
    if (sessionid >= 0 && sessionid < g_SessionNum) {
        InitInfo(sessionid);
    } else {
        LOG_PRINT_ERROR_C("session id is out of range, sid:%d", sessionid);
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
        LOG_PRINT_ERROR_C("Could not open input, %s", filename);
        goto end;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        LOG_PRINT_ERROR_C("Could not find stream information, %s", filename);
        goto end;
    }

    av_dump_format(fmt_ctx, SESSION_TO_INDEX(sessionid), input_filename, 0); // 获取文件的格式 - zhangpeng
    //PrintfInfo(sessionid);

end:
    avformat_close_input(&fmt_ctx);
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }
    av_file_unmap(buffer, buffer_size);

    if (ret < 0) {
        LOG_PRINT_ERROR_C("Error occurred: %s", av_err2str(ret));
        return 1;
    }

    return 0;
}

PUBLIC void tit_store_log(char *line, int indx)
{
    LOG_PRINT_INFO_C("index %d, session %d", indx, INDEX_TO_SESSION(indx));
    if (indx >= OFFSET_OF_SESSION && indx < SESSION_TO_INDEX(g_SessionNum))
    {
        memcpy(g_AudioFormatInfo[INDEX_TO_SESSION(indx)], line, strlen(line));
    }
}


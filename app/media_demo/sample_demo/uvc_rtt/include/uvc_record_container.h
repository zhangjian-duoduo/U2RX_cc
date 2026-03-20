/**
 * @file uvc_record_container.h
 * @brief UVC 录像容器封装头文件
 * @description 支持 AVI 和 MP4 容器格式封装
 */
#ifndef __UVC_RECORD_CONTAINER_H__
#define __UVC_RECORD_CONTAINER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

/**
 * @brief 录像容器类型
 */
typedef enum {
    RECORD_CONTAINER_RAW = 0,   /* 原始流 */
    RECORD_CONTAINER_AVI = 1,   /* AVI 容器 */
    RECORD_CONTAINER_MP4 = 2,   /* MP4 容器 */
    RECORD_CONTAINER_USER = 3   /* 用户自定义 */
} record_container_type_t;

/**
 * @brief 帧索引条目
 */
typedef struct {
    unsigned int offset;           /* 帧数据偏移 */
    unsigned int size;              /* 帧大小 */
    unsigned int is_keyframe;      /* 是否关键帧 */
} frame_index_entry_t;

#define MAX_FRAME_INDEX 3000       /* 最大帧索引数 */

/**
 * @brief 录像上下文
 */
typedef struct {
    int fd;                         /* 文件描述符 */
    char filename[64];             /* 文件名 */
    record_container_type_t type;   /* 容器类型 */
    int width;                     /* 视频宽度 */
    int height;                    /* 视频高度 */
    int fps;                       /* 帧率 */
    int video_type;                /* 视频编码类型 (H264/H265) */
    unsigned int frame_count;       /* 帧计数 */
    unsigned int data_size;         /* 数据大小 */
    int is_keyframe;                /* 是否是关键帧 */
    pthread_mutex_t mutex;         /* 互斥锁 */
    /* 帧索引 */
    frame_index_entry_t frame_index[MAX_FRAME_INDEX];
    unsigned int index_count;
    unsigned int movi_offset;      /* movi数据起始偏移 */
    unsigned int header_size;      /* 头部大小 */
} record_context_t;

/**
 * @brief 初始化录像容器
 * @param filename 文件名
 * @param type 容器类型
 * @param width 视频宽度
 * @param height 视频高度
 * @param fps 帧率
 * @param video_type 视频编码类型
 * @return 录像上下文指针，NULL 表示失败
 */
record_context_t* uvc_record_container_init(const char *filename, record_container_type_t type,
                                              int width, int height, int fps, int video_type);

/**
 * @brief 写入视频帧
 * @param ctx 录像上下文
 * @param data 帧数据
 * @param len 数据长度
 * @param is_keyframe 是否关键帧
 * @return 0 成功，-1 失败
 */
int uvc_record_container_write_frame(record_context_t *ctx, const unsigned char *data, int len, int is_keyframe);

/**
 * @brief 关闭录像容器
 * @param ctx 录像上下文
 */
void uvc_record_container_close(record_context_t *ctx);

#endif /* __UVC_RECORD_CONTAINER_H__ */

/**
 * @file uvc_record_container.c
 * @brief UVC 录像容器封装实现
 * @description 支持 AVI 和 MP4 容器格式封装
 */
#include "uvc_record_container.h"
#include "uvc_feature_config.h"

/* AVI 相关常量 */
#define AVI_MAX_SIZE_2GB  0x7FFFFFFF

/* AVI 头标识 */
#define AVI_FOURCC_RIFF  0x46464952  /* "RIFF" */
#define AVI_FOURCC_AVI   0x49564120  /* "AVI " */
#define AVI_FOURCC_LIST 0x5453494C  /* "LIST" */
#define AVI_FOURCC_hdrl 0x6C726468  /* "hdrl" */
#define AVI_FOURCC_avih 0x68697661  /* "avih" */
#define AVI_FOURCC_strl 0x6C727473  /* "strl" */
#define AVI_FOURCC_strh 0x68727473  /* "strh" */
#define AVI_FOURCC_strf 0x66727473  /* "strf" */
#define AVI_FOURCC_movi 0x69766F6D  /* "movi" */
#define AVI_FOURCC_00db 0x62643030  /* "00db" - 未压缩视频帧 */
#define AVI_FOURCC_00dc 0x63643030  /* "00dc" - 压缩视频帧 */
#define AVI_FOURCC_00ix 0x69783030  /* "00ix" - 索引 */
#define AVI_FOURCC_idx1 0x31786469  /* "idx1" */

/* H264 NALU 类型 */
#define NALU_TYPE_NON_IDR 1
#define NALU_TYPE_IDR 5
#define NALU_TYPE_SEI 6
#define NALU_TYPE_SPS 7
#define NALU_TYPE_PPS 8

/* MP4 原子类型 */
#define MP4_FOURCC_ftyp 0x66747970  /* "ftyp" */
#define MP4_FOURCC_mdat 0x6D646174  /* "mdat" */
#define MP4_FOURCC_moov 0x6D6F6F76  /* "moov" */
#define MP4_FOURCC_mvhd 0x6D766864  /* "mvhd" */
#define MP4_FOURCC_trak 0x7472616B  /* "trak" */
#define MP4_FOURCC_tkhd 0x746B6864  /* "tkhd" */
#define MP4_FOURCC_mdia 0x6D646961  /* "mdia" */
#define MP4_FOURCC_mdhd 0x6D646864  /* "mdhd" */
#define MP4_FOURCC_hdlr 0x6864726C  /* "hdlr" */
#define MP4_FOURCC_minf 0x6D696E66  /* "minf" */
#define MP4_FOURCC_stbl 0x7374626C  /* "stbl" */
#define MP4_FOURCC_stsd 0x73747364  /* "stsd" */
#define MP4_FOURCC_stts 0x73747473  /* "stts" */
#define MP4_FOURCC_stsc 0x73747363  /* "stsc" */
#define MP4_FOURCC_stsz 0x7374737A  /* "stsz" */
#define MP4_FOURCC_stco 0x7374636F  /* "stco" */
#define MP4_FOURCC_avc1 0x61637631  /* "avc1" */
#define MP4_FOURCC_avcC 0x61637643  /* "avcC" */

/* 视频编码类型 */
#define VIDEO_TYPE_H264  0
#define VIDEO_TYPE_H265  1

/**
 * @brief 写入 32 位整数 (小端序)
 */
static void write_uint32(unsigned char *buf, unsigned int val)
{
    buf[0] = (val >> 0) & 0xFF;
    buf[1] = (val >> 8) & 0xFF;
    buf[2] = (val >> 16) & 0xFF;
    buf[3] = (val >> 24) & 0xFF;
}

/**
 * @brief 写入 16 位整数 (小端序)
 */
static void write_uint16(unsigned char *buf, unsigned short val)
{
    buf[0] = (val >> 0) & 0xFF;
    buf[1] = (val >> 8) & 0xFF;
}

/**
 * @brief 初始化 AVI 容器
 */
static record_context_t* avi_container_init(const char *filename,
                                            int width, int height, int fps, int video_type)
{
    record_context_t *ctx = (record_context_t *)malloc(sizeof(record_context_t));
    if (!ctx) return NULL;

    memset(ctx, 0, sizeof(record_context_t));
    strncpy(ctx->filename, filename, sizeof(ctx->filename) - 1);
    ctx->type = RECORD_CONTAINER_AVI;
    ctx->width = width;
    ctx->height = height;
    ctx->fps = fps;
    ctx->video_type = video_type;
    pthread_mutex_init(&ctx->mutex, NULL);

    /* 打开文件 */
    ctx->fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (ctx->fd < 0) {
        free(ctx);
        return NULL;
    }

    /* 写入 AVI 头 (占位，后面会更新) */
    unsigned char avi_header[4096];
    memset(avi_header, 0, sizeof(avi_header));

    /* RIFF header */
    int offset = 0;
    write_uint32(avi_header + offset, AVI_FOURCC_RIFF); offset += 4;  /* "RIFF" */
    write_uint32(avi_header + offset, 0); offset += 4;  /* 文件大小 - 8 */
    write_uint32(avi_header + offset, AVI_FOURCC_AVI); offset += 4;  /* "AVI " */

    /* hdrl LIST */
    write_uint32(avi_header + offset, AVI_FOURCC_LIST); offset += 4;
    write_uint32(avi_header + offset, 0); offset += 4;  /* hdrl size */
    write_uint32(avi_header + offset, AVI_FOURCC_hdrl); offset += 4;  /* "hdrl" */

    /* avih chunk */
    write_uint32(avi_header + offset, AVI_FOURCC_avih); offset += 4;
    write_uint32(avi_header + offset, 56); offset += 4;  /* chunk size */
    unsigned int microsec_per_frame = 1000000 / fps;
    write_uint32(avi_header + offset, microsec_per_frame); offset += 4;  /* dwMicroSecPerFrame */
    write_uint32(avi_header + offset, width * height * fps * 2); offset += 4;  /* dwMaxBytesPerSec */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwPaddingGranularity */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwFlags */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwTotalFrames */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwInitialFrames */
    write_uint32(avi_header + offset, 1); offset += 4;  /* dwStreams */
    write_uint32(avi_header + offset, width * height * 3); offset += 4;  /* dwSuggestedBufferSize */
    write_uint32(avi_header + offset, width); offset += 4;  /* dwWidth */
    write_uint32(avi_header + offset, height); offset += 4;  /* dwHeight */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwReserved */
    write_uint32(avi_header + offset, 0); offset += 4;
    write_uint32(avi_header + offset, 0); offset += 4;
    write_uint32(avi_header + offset, 0); offset += 4;

    /* strl LIST */
    write_uint32(avi_header + offset, AVI_FOURCC_LIST); offset += 4;
    write_uint32(avi_header + offset, 116); offset += 4;  /* strl size */
    write_uint32(avi_header + offset, AVI_FOURCC_strl); offset += 4;  /* "strl" */

    /* strh chunk */
    write_uint32(avi_header + offset, AVI_FOURCC_strh); offset += 4;
    write_uint32(avi_header + offset, 64); offset += 4;  /* chunk size */
    write_uint32(avi_header + offset, 0x73646976); offset += 4;  /* "vids" */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwCodec */
    write_uint16(avi_header + offset, 0); offset += 2;  /* wFlags */
    write_uint16(avi_header + offset, 0); offset += 2;  /* wPriority */
    write_uint16(avi_header + offset, 0); offset += 2;  /* wLanguage */
    write_uint16(avi_header + offset, 0); offset += 2;  /* wInitialFrames */
    write_uint32(avi_header + offset, 1); offset += 4;  /* dwScale */
    write_uint32(avi_header + offset, fps); offset += 4;  /* dwRate */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwStart */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwLength */
    write_uint32(avi_header + offset, width * height * fps); offset += 4;  /* dwSuggestedBufferSize */
    write_uint32(avi_header + offset, 0xFFFFFFFF); offset += 4;  /* dwQuality */
    write_uint32(avi_header + offset, 0); offset += 4;  /* dwSampleSize */
    write_uint16(avi_header + offset, 0); offset += 2;  /* rcFrame.left */
    write_uint16(avi_header + offset, 0); offset += 2;  /* rcFrame.top */
    write_uint16(avi_header + offset, width); offset += 2;  /* rcFrame.right */
    write_uint16(avi_header + offset, height); offset += 2;  /* rcFrame.bottom */

    /* strf chunk (BITMAPINFOHEADER) */
    write_uint32(avi_header + offset, AVI_FOURCC_strf); offset += 4;
    write_uint32(avi_header + offset, 40); offset += 4;  /* chunk size */
    write_uint32(avi_header + offset, 40); offset += 4;  /* biSize */
    write_uint32(avi_header + offset, width); offset += 4;  /* biWidth */
    write_uint32(avi_header + offset, height); offset += 4;  /* biHeight */
    write_uint16(avi_header + offset, 1); offset += 2;  /* biPlanes */
    write_uint16(avi_header + offset, 24); offset += 2;  /* biBitCount */
    write_uint32(avi_header + offset, 0); offset += 4;  /* biCompression - H264 使用自定义解码器 */
    write_uint32(avi_header + offset, width * height * 3); offset += 4;  /* biSizeImage */
    write_uint32(avi_header + offset, 0); offset += 4;  /* biXPelsPerMeter */
    write_uint32(avi_header + offset, 0); offset += 4;  /* biYPelsPerMeter */
    write_uint32(avi_header + offset, 0); offset += 4;  /* biClrUsed */
    write_uint32(avi_header + offset, 0); offset += 4;  /* biClrImportant */

    /* movi LIST */
    write_uint32(avi_header + offset, AVI_FOURCC_LIST); offset += 4;
    write_uint32(avi_header + offset, 0); offset += 4;  /* movi size */
    write_uint32(avi_header + offset, AVI_FOURCC_movi); offset += 4;  /* "movi" */

    /* 写入头部 */
    write(ctx->fd, avi_header, offset);

    printf("[Record] AVI container initialized: %s\n", filename);

    return ctx;
}

/**
 * @brief 写入 AVI 视频帧
 */
static int avi_container_write_frame(record_context_t *ctx, const unsigned char *data, int len, int is_keyframe)
{
    if (!ctx || ctx->fd < 0) return -1;

    pthread_mutex_lock(&ctx->mutex);

    /* 构建 AVI chunk */
    unsigned char chunk[16];
    int chunk_size = 8 + len;  /* chunk header + data */
    chunk_size = (chunk_size + 1) & ~1;  /* 2字节对齐 */

    /* chunk type: 00dc (压缩视频) 或 00db (未压缩) */
    if (is_keyframe) {
        write_uint32(chunk, AVI_FOURCC_00dc);  /* 关键帧用 00dc */
    } else {
        write_uint32(chunk, 0x62643030);  /* 00db - P帧 */
    }
    write_uint32(chunk + 4, len);  /* chunk size */
    write(ctx->fd, chunk, 8);
    write(ctx->fd, data, len);
    if (len & 1) {
        char pad = 0;
        write(ctx->fd, &pad, 1);  /* 填充字节 */
    }

    ctx->frame_count++;
    ctx->data_size += len;

    pthread_mutex_unlock(&ctx->mutex);

    return 0;
}

/**
 * @brief 关闭 AVI 容器
 */
static void avi_container_close(record_context_t *ctx)
{
    if (!ctx || ctx->fd < 0) return;

    pthread_mutex_lock(&ctx->mutex);

    /* 更新 AVI 头中的帧数和文件大小 */
    lseek(ctx->fd, 0, SEEK_SET);

    unsigned char avi_header[256];
    memset(avi_header, 0, sizeof(avi_header));

    int offset = 0;
    write_uint32(avi_header + offset, AVI_FOURCC_RIFF); offset += 4;
    unsigned int file_size = 4 + 4 + 4 + 4 + 56 + 4 + 4 + 4 + 116 + 4 + 4 + 64 + 40 + 4 + 4 + 4 + 4 + 8 + 8 + ctx->data_size + 4 + 4 + 4 + 4;
    write_uint32(avi_header + offset, file_size - 8); offset += 4;
    write_uint32(avi_header + offset, AVI_FOURCC_AVI); offset += 4;

    /* 更新 hdrl LIST 大小 */
    write_uint32(avi_header + offset, AVI_FOURCC_LIST); offset += 4;
    write_uint32(avi_header + offset, 4 + 56 + 4 + 4 + 116); offset += 4;
    write_uint32(avi_header + offset, AVI_FOURCC_hdrl); offset += 4;

    /* 更新 avih chunk */
    write_uint32(avi_header + offset, AVI_FOURCC_avih); offset += 4;
    write_uint32(avi_header + offset, 56); offset += 4;
    unsigned int microsec_per_frame = 1000000 / ctx->fps;
    write_uint32(avi_header + offset, microsec_per_frame); offset += 4;

    lseek(ctx->fd, 0, SEEK_SET);
    read(ctx->fd, avi_header, 256);

    /* 回到文件开头更新 */
    lseek(ctx->fd, 0, SEEK_SET);

    /* 查找并更新帧数位置 */
    unsigned char *p = avi_header;
    while (p < avi_header + 200) {
        if (*(unsigned int*)p == AVI_FOURCC_avih) {
            p += 8;
            write_uint32(p, ctx->frame_count);  /* dwTotalFrames */
            break;
        }
        p++;
    }

    write(ctx->fd, avi_header, 192);

    close(ctx->fd);
    ctx->fd = -1;

    pthread_mutex_unlock(&ctx->mutex);

    printf("[Record] AVI container closed: %s, frames=%u, size=%u\n",
           ctx->filename, ctx->frame_count, ctx->data_size);
}

/**
 * @brief 初始化 MP4 容器 (简化版，使用 AVCC 格式)
 */
static record_context_t* mp4_container_init(const char *filename,
                                             int width, int height, int fps, int video_type)
{
    record_context_t *ctx = (record_context_t *)malloc(sizeof(record_context_t));
    if (!ctx) return NULL;

    memset(ctx, 0, sizeof(record_context_t));
    strncpy(ctx->filename, filename, sizeof(ctx->filename) - 1);
    ctx->type = RECORD_CONTAINER_MP4;
    ctx->width = width;
    ctx->height = height;
    ctx->fps = fps;
    ctx->video_type = video_type;
    pthread_mutex_init(&ctx->mutex, NULL);

    /* 打开文件 */
    ctx->fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (ctx->fd < 0) {
        free(ctx);
        return NULL;
    }

    /* 写入空的 ftyp 和 mdat 头，后面会更新 */
    unsigned char mp4_header[1024];
    memset(mp4_header, 0, sizeof(mp4_header));

    /* ftyp box */
    int offset = 0;
    write_uint32(mp4_header + offset, 32); offset += 4;  /* box size */
    write_uint32(mp4_header + offset, MP4_FOURCC_ftyp); offset += 4;  /* "ftyp" */
    write_uint32(mp4_header + offset, 0x00000000); offset += 4;  /* major brand */
    write_uint32(mp4_header + offset, 0x00000000); offset += 4;  /* minor version */
    write_uint32(mp4_header + offset, 0x00000001); offset += 4;  /* compatible brand */
    write_uint32(mp4_header + offset, 0x00000002); offset += 4;
    write_uint32(mp4_header + offset, 0x69736F6D); offset += 4;  /* "isom" */
    write_uint32(mp4_header + offset, 0x6D703431); offset += 4;  /* "mp41" */

    /* mdat box (空) */
    write_uint32(mp4_header + offset, 16); offset += 4;  /* box size */
    write_uint32(mp4_header + offset, MP4_FOURCC_mdat); offset += 4;  /* "mdat" */

    write(ctx->fd, mp4_header, offset);

    printf("[Record] MP4 container initialized: %s\n", filename);

    return ctx;
}

/**
 * @brief 写入 MP4 视频帧
 */
static int mp4_container_write_frame(record_context_t *ctx, const unsigned char *data, int len, int is_keyframe)
{
    if (!ctx || ctx->fd < 0) return -1;

    pthread_mutex_lock(&ctx->mutex);

    /* 写入 NALU 长度 + 数据 */
    unsigned char len_buf[4];
    write_uint32(len_buf, len);
    write(ctx->fd, len_buf, 4);
    write(ctx->fd, data, len);

    ctx->frame_count++;
    ctx->data_size += len;

    pthread_mutex_unlock(&ctx->mutex);

    return 0;
}

/**
 * @brief 关闭 MP4 容器
 */
static void mp4_container_close(record_context_t *ctx)
{
    if (!ctx || ctx->fd < 0) return;

    pthread_mutex_lock(&ctx->mutex);

    /* 更新 mdat 大小 */
    lseek(ctx->fd, 20, SEEK_SET);  /* 跳到 mdat size 位置 */
    unsigned char mdat_size[4];
    write_uint32(mdat_size, 16 + ctx->data_size);
    write(ctx->fd, mdat_size, 4);

    close(ctx->fd);
    ctx->fd = -1;

    pthread_mutex_unlock(&ctx->mutex);

    printf("[Record] MP4 container closed: %s, frames=%u, size=%u\n",
           ctx->filename, ctx->frame_count, ctx->data_size);
}

/**
 * @brief 初始化录像容器
 */
record_context_t* uvc_record_container_init(const char *filename, record_container_type_t type,
                                            int width, int height, int fps, int video_type)
{
    switch (type) {
        case RECORD_CONTAINER_AVI:
            return avi_container_init(filename, width, height, fps, video_type);
        case RECORD_CONTAINER_MP4:
            return mp4_container_init(filename, width, height, fps, video_type);
        default:
            printf("[Record] Unknown container type: %d\n", type);
            return NULL;
    }
}

/**
 * @brief 写入视频帧
 */
int uvc_record_container_write_frame(record_context_t *ctx, const unsigned char *data, int len, int is_keyframe)
{
    if (!ctx) return -1;

    switch (ctx->type) {
        case RECORD_CONTAINER_AVI:
            return avi_container_write_frame(ctx, data, len, is_keyframe);
        case RECORD_CONTAINER_MP4:
            return mp4_container_write_frame(ctx, data, len, is_keyframe);
        default:
            return -1;
    }
}

/**
 * @brief 关闭录像容器
 */
void uvc_record_container_close(record_context_t *ctx)
{
    if (!ctx) return;

    switch (ctx->type) {
        case RECORD_CONTAINER_AVI:
            avi_container_close(ctx);
            break;
        case RECORD_CONTAINER_MP4:
            mp4_container_close(ctx);
            break;
        default:
            break;
    }

    pthread_mutex_destroy(&ctx->mutex);
    free(ctx);
}

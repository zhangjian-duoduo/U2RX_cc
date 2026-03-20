/**
 * @file uvc_sd_record.c
 * @brief UVC SD 卡录像功能实现
 */
#include "uvc_rtt/include/uvc_sd_record.h"
#include "uvc_rtt/include/uvc_feature_config.h"

#if UVC_ENABLE_SD_RECORD

#include <dfs.h>
#include <dfs_fs.h>
#include "libdmc.h"

/* 根据录像格式选择不同的实现 */
#if UVC_SD_RECORD_FORMAT == 0
/* 原始流格式 - 直接保存 H264 数据 */
#define RECORD_CONTEXT void*
static int g_record_initialized = 0;
static int g_video_width = 1920;
static int g_video_height = 1080;
static int g_video_fps = 30;
static int g_record_fd = -1;

#elif UVC_SD_RECORD_FORMAT == 1
#if UVC_USE_THIRD_PARTY_AVI
/* AVI 格式 - 使用第三方 libfhavi 库 */
#include "FHAVI.h"
static FHHANDLE g_fhavi_handle = NULL;
static int g_video_width = 1920;
static int g_video_height = 1080;
static int g_video_fps = 30;
static int g_video_type = 0;  /* 0: H264, 1: H265 */
static unsigned char *g_sps_data = NULL;
static int g_sps_len = 0;
static unsigned char *g_pps_data = NULL;
static int g_pps_len = 0;
static int g_sps_pps_extracted = 0;  /* SPS/PPS是否已提取 */

/**
 * @brief 从H264数据中提取SPS和PPS
 * @param data H264帧数据
 * @param len 数据长度
 * @return 0 成功, -1 失败
 */
static int extract_sps_pps_from_h264(unsigned char *data, int len)
{
    unsigned char *ptr = data;
    unsigned char *sps_start = NULL, *pps_start = NULL;
    int sps_found = 0, pps_found = 0;

    // 查找SPS (NAL type 7) 和 PPS (NAL type 8)
    while (ptr < data + len - 5) {
        // 跳过start code
        if (ptr[0] == 0 && ptr[1] == 0 && (ptr[2] == 1 || (ptr[2] == 0 && ptr[3] == 1))) {
            // 确定start code长度
            int sc_len = (ptr[2] == 1) ? 3 : 4;
            unsigned char *nal_start = ptr + sc_len;
            if (nal_start >= data + len) break;

            // 获取NAL类型 (低5位)
            int nal_type = nal_start[0] & 0x1F;

            // 找到下一个NAL单元
            unsigned char *next_nal = NULL;
            for (unsigned char *p = nal_start + 1; p < data + len - 4; p++) {
                if (p[0] == 0 && p[1] == 0 && (p[2] == 1 || (p[2] == 0 && p[3] == 1))) {
                    next_nal = p;
                    break;
                }
            }

            int nal_len = next_nal ? (next_nal - nal_start) : (data + len - nal_start);

            if (nal_type == 7 && !sps_found) {
                // SPS found
                sps_start = nal_start;
                g_sps_len = nal_len;
                sps_found = 1;
            } else if (nal_type == 8 && !pps_found) {
                // PPS found
                pps_start = nal_start;
                g_pps_len = nal_len;
                pps_found = 1;
            }

            if (sps_found && pps_found) break;
            ptr = next_nal ? next_nal : data + len;
        } else {
            ptr++;
        }
    }

    if (sps_found && pps_found) {
        // 复制SPS和PPS数据
        g_sps_data = (unsigned char *)malloc(g_sps_len);
        g_pps_data = (unsigned char *)malloc(g_pps_len);
        if (g_sps_data && g_pps_data) {
            memcpy(g_sps_data, sps_start, g_sps_len);
            memcpy(g_pps_data, pps_start, g_pps_len);
            g_sps_pps_extracted = 1;
            return 0;
        }
    }

    return -1;
}

#else
/* AVI 格式 - 使用内置简化版代码 */
#include "uvc_record_container.h"
static record_context_t *g_record_ctx = NULL;
static int g_video_width = 1920;
static int g_video_height = 1080;
static int g_video_fps = 30;
static int g_video_type = 0;  /* 0: H264, 1: H265 */
#endif

#elif UVC_SD_RECORD_FORMAT == 2
/* MP4 格式 - 使用新的容器封装 */
#include "uvc_record_container.h"
static record_context_t *g_record_ctx = NULL;
static int g_video_width = 1920;
static int g_video_height = 1080;
static int g_video_fps = 30;

#else
/* 用户自定义格式 */
#include "librecord.h"
#define RECORD_CONTEXT void*
static int g_record_initialized = 0;

#endif

static int g_sd_mounted = 0;
static int g_recording = 0;
static int g_stopping = 0;  /* 停止标志，防止在停止过程中写入帧 */
static pthread_mutex_t g_record_mutex = PTHREAD_MUTEX_INITIALIZER;
static char g_record_filename[64] = {0};

/**
 * @brief 检查 SD 卡是否已挂载
 */
int uvc_sd_record_is_mounted(void)
{
    return g_sd_mounted;
}

/**
 * @brief 挂载 SD 卡
 */
static int mount_sdcard(void)
{
    int ret;

    // 先创建挂载点目录
    ret = mkdir(UVC_SD_MOUNT_PATH, 0);
    if (ret != 0 && errno != EEXIST)
    {
        printf("[SD Record] Failed to create mount point %s: %d\n", UVC_SD_MOUNT_PATH, errno);
    }

    // 尝试挂载 SD 卡到 /mnt/sdcard
    ret = dfs_mount("mmcblk0p1", UVC_SD_MOUNT_PATH, "elm", 0, NULL);
    if (ret != 0)
    {
        // 如果分区挂载失败，尝试直接挂载整个设备
        printf("[SD Record] Partition mount failed, trying whole device...\n");
        ret = dfs_mount("mmcblk0", UVC_SD_MOUNT_PATH, "elm", 0, NULL);
        if (ret != 0)
        {
            printf("[SD Record] Failed to mount SD card: %d\n", ret);
            return -1;
        }
    }

    g_sd_mounted = 1;
    printf("[SD Record] SD card mounted successfully at %s\n", UVC_SD_MOUNT_PATH);

    // 创建录像目录
    ret = mkdir(UVC_SD_RECORD_PATH, 0);
    if (ret != 0 && errno != EEXIST)
    {
        printf("[SD Record] Failed to create directory %s: %d\n", UVC_SD_RECORD_PATH, errno);
    }
    else
    {
        printf("[SD Record] Directory ready: %s\n", UVC_SD_RECORD_PATH);
    }

    return 0;
}

/**
 * @brief 卸载 SD 卡
 */
static void unmount_sdcard(void)
{
    if (g_sd_mounted)
    {
        dfs_unmount("/");
        g_sd_mounted = 0;
        printf("[SD Record] SD card unmounted\n");
    }
}

/**
 * @brief 根据格式生成文件名
 */
static void generate_filename(char *filename, int len)
{
    time_t now;
    struct tm *tm_now;
    struct timeval tv;

    time(&now);
    tm_now = localtime(&now);
    gettimeofday(&tv, NULL);

#if UVC_SD_RECORD_FORMAT == 0
    /* 原始流 - 使用 .h264 扩展名 */
    /* 处理根目录情况，避免双斜杠 */
    if (UVC_SD_RECORD_PATH[0] == '/' && UVC_SD_RECORD_PATH[1] == '\0') {
        snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d.h264",
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    } else {
        snprintf(filename, len, "%s/%04d%02d%02d_%02d%02d%02d.h264",
                 UVC_SD_RECORD_PATH,
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    }
#elif UVC_SD_RECORD_FORMAT == 1
    /* AVI 格式 */
    /* 处理根目录情况，避免双斜杠 */
    if (UVC_SD_RECORD_PATH[0] == '/' && UVC_SD_RECORD_PATH[1] == '\0') {
        snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d.avi",
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    } else {
        snprintf(filename, len, "%s/%04d%02d%02d_%02d%02d%02d.avi",
                 UVC_SD_RECORD_PATH,
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    }
#elif UVC_SD_RECORD_FORMAT == 2
    /* MP4 格式 */
    /* 处理根目录情况，避免双斜杠 */
    if (UVC_SD_RECORD_PATH[0] == '/' && UVC_SD_RECORD_PATH[1] == '\0') {
        snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d.mp4",
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    } else {
        snprintf(filename, len, "%s/%04d%02d%02d_%02d%02d%02d.mp4",
                 UVC_SD_RECORD_PATH,
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    }
#else
    /* 用户自定义格式 */
    /* 处理根目录情况，避免双斜杠 */
    if (UVC_SD_RECORD_PATH[0] == '/' && UVC_SD_RECORD_PATH[1] == '\0') {
        snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d",
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    } else {
        snprintf(filename, len, "%s/%04d%02d%02d_%02d%02d%02d",
                 UVC_SD_RECORD_PATH,
                 tm_now->tm_year + 1900,
                 tm_now->tm_mon + 1,
                 tm_now->tm_mday,
                 tm_now->tm_hour,
                 tm_now->tm_min,
                 tm_now->tm_sec);
    }
             tm_now->tm_mon + 1,
             tm_now->tm_mday,
             tm_now->tm_hour,
             tm_now->tm_min,
             tm_now->tm_sec);
             tm_now->tm_year + 1900,
             tm_now->tm_mon + 1,
             tm_now->tm_mday,
             tm_now->tm_hour,
             tm_now->tm_min,
             tm_now->tm_sec);
#endif
}

/**
 * @brief 初始化录像容器
 */
static int init_container(void)
{
#if UVC_SD_RECORD_FORMAT == 0
    /* 原始流格式 - 直接保存 H264 数据 */
    g_record_fd = open(g_record_filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (g_record_fd < 0) {
        printf("[SD Record] Cannot create file: %s, errno=%d\n", g_record_filename, errno);
        return -1;
    }
    g_record_initialized = 1;
    printf("[SD Record] Raw H264 file created: %s\n", g_record_filename);
    return 0;

#elif UVC_SD_RECORD_FORMAT == 1
#if UVC_USE_THIRD_PARTY_AVI
    /* AVI 格式 - 使用第三方 libfhavi 库 */
    printf("[SD Record] FHAVI_Start: %s, g_video_type=%d\n", g_record_filename, g_video_type);

    FHAVI_Config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.videoWidth = g_video_width;
    cfg.videoHeight = g_video_height;
    cfg.videoRate = g_video_fps;
    cfg.videoMaxBitrate = 0;
    cfg.audioSamplerate = 0;
    cfg.audioBitWidth = 0;
    cfg.audioMaxBitrate = 0;
    cfg.audioTrack = 0;
    cfg.streamType = (FHU8)g_video_type;  /* 0: H264, 1: H265 */
    cfg.isAudioFrame = 0;  /* 无音频 */
    cfg.reserved = 0;
    /* SPS/PPS for avcC configuration */
    cfg.spsData = g_sps_data;
    cfg.spsLen = g_sps_len;
    cfg.ppsData = g_pps_data;
    cfg.ppsLen = g_pps_len;

    printf("[SD Record] FHAVI_Start cfg: size=%d, streamType=%d, isAudioFrame=%d, w=%d, h=%d, fps=%d\n",
           (int)sizeof(cfg), cfg.streamType, cfg.isAudioFrame, cfg.videoWidth, cfg.videoHeight, cfg.videoRate);
    printf("[SD Record] FHAVI_Start cfg: audioTrack=%d, spsLen=%d, ppsLen=%d\n",
           cfg.audioTrack, cfg.spsLen, cfg.ppsLen);

    int ret = FHAVI_Start(&g_fhavi_handle, g_record_filename, &cfg);
    if (ret != FHAVI_OK)
    {
        printf("[SD Record] FHAVI_Start failed: %d\n", ret);
        return -1;
    }
    printf("[SD Record] Third-party AVI started: %s\n", g_record_filename);
    return 0;
#else
    /* AVI 格式 - 使用内置简化版代码 */
    g_record_ctx = uvc_record_container_init(g_record_filename,
                                             (record_container_type_t)UVC_SD_RECORD_FORMAT,
                                             g_video_width, g_video_height,
                                             g_video_fps, g_video_type);
    if (!g_record_ctx)
    {
        printf("[SD Record] Failed to init container\n");
        return -1;
    }
    return 0;
#endif

#elif UVC_SD_RECORD_FORMAT == 2
    /* MP4 格式 */
    g_record_ctx = uvc_record_container_init(g_record_filename,
                                             (record_container_type_t)UVC_SD_RECORD_FORMAT,
                                             g_video_width, g_video_height,
                                             g_video_fps, g_video_type);
    if (!g_record_ctx)
    {
        printf("[SD Record] Failed to init container\n");
        return -1;
    }
    return 0;

#else
    /* 用户自定义格式 */
    if (!g_record_initialized)
    {
        librecord_init();
        g_record_initialized = 1;
    }
    return 0;
#endif
}

/**
 * @brief 关闭录像容器
 */
static void close_container(void)
{
    /* 设置停止标志，防止在关闭过程中视频流线程继续写入 */
    g_stopping = 1;

#if UVC_SD_RECORD_FORMAT == 0
    /* 原始流格式 */
    if (g_record_fd >= 0) {
        close(g_record_fd);
        g_record_fd = -1;
        printf("[SD Record] Raw H264 file closed\n");
    }
    g_record_initialized = 0;

#elif UVC_SD_RECORD_FORMAT == 1
#if UVC_USE_THIRD_PARTY_AVI
    /* AVI 格式 - 使用第三方 libfhavi 库 */
    if (g_fhavi_handle != NULL)
    {
        FHAVI_Stop(g_fhavi_handle);
        g_fhavi_handle = NULL;
    }
#else
    /* AVI 格式 - 使用内置简化版代码 */
    if (g_record_ctx)
    {
        uvc_record_container_close(g_record_ctx);
        g_record_ctx = NULL;
    }
#endif

#elif UVC_SD_RECORD_FORMAT == 2
    /* MP4 格式 */
    if (g_record_ctx)
    {
        uvc_record_container_close(g_record_ctx);
        g_record_ctx = NULL;
    }

#else
    /* 用户自定义格式 */
    if (g_record_initialized)
    {
        librecord_uninit();
        g_record_initialized = 0;
    }
#endif
}

/**
 * @brief 写视频帧
 */
static int write_frame(int stream_id, int media_type, unsigned char *frame_data, int frame_len, int is_keyframe)
{
    /* 检查SD卡是否仍然挂载，如果被拔出则停止录像 */
    if (!g_sd_mounted && g_recording)
    {
        g_recording = 0;
        printf("[SD Record] SD card removed, stopping recording\n");
    }

#if UVC_SD_RECORD_FORMAT == 0
    /* 原始流格式 - 直接写入文件 */
    if (g_record_fd >= 0) {
        int ret = write(g_record_fd, frame_data, frame_len);
        if (ret < 0) {
            printf("[SD Record] Write failed, stopping recording\n");
            g_recording = 0;
        }
    }
    return 0;

#elif UVC_SD_RECORD_FORMAT == 1
#if UVC_USE_THIRD_PARTY_AVI
    /* AVI 格式 - 使用第三方 libfhavi 库 */
    /* 检查停止标志，防止在FHAVI_Stop后访问已释放的内存 */
    if (g_stopping)
    {
        return 0;
    }

    /* 尝试从第一帧提取SPS/PPS（仅对H264有效） */
    if (!g_sps_pps_extracted && media_type == DMC_MEDIA_TYPE_H264 && is_keyframe) {
        extract_sps_pps_from_h264(frame_data, frame_len);
    }

    if (g_fhavi_handle)
    {
        FHU8 frameType = is_keyframe ? FHAVI_FRAME_TYPE_I : FHAVI_FRAME_TYPE_P;
        int ret = FHAVI_Commit(g_fhavi_handle, frameType, frame_len, frame_data);
        if (ret != FHAVI_OK) {
            printf("[SD Record] FHAVI_Commit failed: %d, stopping\n", ret);
            g_recording = 0;
        }
    }
    return 0;
#else
    /* AVI 格式 - 使用内置简化版代码 */
    if (g_record_ctx)
    {
        uvc_record_container_write_frame(g_record_ctx, frame_data, frame_len, is_keyframe);
    }
    return 0;
#endif

#elif UVC_SD_RECORD_FORMAT == 2
    /* MP4 格式 */
    if (g_record_ctx)
    {
        uvc_record_container_write_frame(g_record_ctx, frame_data, frame_len, is_keyframe);
    }
    return 0;

#else
    /* 用户自定义格式 */
    int subtype = is_keyframe ? DMC_MEDIA_SUBTYPE_IFRAME : DMC_MEDIA_SUBTYPE_PFRAME;
    librecord_save(stream_id, media_type, subtype, frame_data, frame_len);
    return 0;
#endif
}

/**
 * @brief 初始化 SD 卡录像模块
 */
int uvc_sd_record_init(void)
{
    // 尝试挂载 SD 卡
    mount_sdcard();

    printf("[SD Record] Init done, SD mounted: %d, format: %d\n", g_sd_mounted, UVC_SD_RECORD_FORMAT);
    return 0;
}

/**
 * @brief 设置视频参数 (需要在开始录像前调用)
 */
void uvc_sd_record_set_video_params(int width, int height, int fps)
{
    g_video_width = width;
    g_video_height = height;
    g_video_fps = fps;
    printf("[SD Record] Video params: %dx%d @ %d fps\n", width, height, fps);
}

/**
 * @brief 设置视频编码类型 (需要在开始录像前调用, 仅第三方AVI库有效)
 * @param type 0: H264, 1: H265
 */
void uvc_sd_record_set_video_type(int type)
{
#if UVC_USE_THIRD_PARTY_AVI
    g_video_type = type;
    printf("[SD Record] Video type: %s\n", type == 0 ? "H264" : "H265");
#else
    (void)type;  /* 防止警告 */
#endif
}

/**
 * @brief 设置 H264/H265 SPS/PPS 数据 (需要在开始录像前调用, 仅第三方AVI库有效)
 */
void uvc_sd_record_set_sps_pps(unsigned char *sps, int sps_len, unsigned char *pps, int pps_len)
{
#if UVC_USE_THIRD_PARTY_AVI
    g_sps_data = sps;
    g_sps_len = sps_len;
    g_pps_data = pps;
    g_pps_len = pps_len;
    printf("[SD Record] SPS/PPS set: sps=%d bytes, pps=%d bytes\n", sps_len, pps_len);
#else
    (void)sps; (void)sps_len; (void)pps; (void)pps_len;
#endif
}

/**
 * @brief 开始录像
 */
int uvc_sd_record_start(void)
{
    pthread_mutex_lock(&g_record_mutex);

    if (g_recording)
    {
        printf("[SD Record] Already recording\n");
        pthread_mutex_unlock(&g_record_mutex);
        return 0;
    }

    // 检查 SD 卡
    if (!g_sd_mounted)
    {
        printf("[SD Record] SD card not mounted, cannot start recording\n");
        pthread_mutex_unlock(&g_record_mutex);
        return -1;
    }

    // 生成文件名
    generate_filename(g_record_filename, sizeof(g_record_filename));

    /* 重置SPS/PPS提取标志，确保每次录像都从头开始 (仅第三方库需要) */
#if UVC_USE_THIRD_PARTY_AVI
    g_sps_pps_extracted = 0;
    g_sps_len = 0;
    g_pps_len = 0;
#endif

    // 初始化容器
    if (init_container() != 0)
    {
        printf("[SD Record] Failed to init container\n");
        pthread_mutex_unlock(&g_record_mutex);
        return -1;
    }

    /* 重置停止标志 */
    g_stopping = 0;
    g_recording = 1;
    printf("[SD Record] Start recording: %s\n", g_record_filename);

    pthread_mutex_unlock(&g_record_mutex);
    return 0;
}

/**
 * @brief 停止录像
 */
int uvc_sd_record_stop(void)
{
    pthread_mutex_lock(&g_record_mutex);

    if (!g_recording)
    {
        pthread_mutex_unlock(&g_record_mutex);
        return 0;
    }

    // 关闭容器
    close_container();

    g_recording = 0;
    g_stopping = 0;  /* 重置停止标志，允许下次录像 */
    printf("[SD Record] Stop recording\n");

    pthread_mutex_unlock(&g_record_mutex);
    return 0;
}

/**
 * @brief 销毁 SD 卡录像模块
 */
void uvc_sd_record_deinit(void)
{
    if (g_recording)
    {
        uvc_sd_record_stop();
    }

    unmount_sdcard();

    printf("[SD Record] Deinit done\n");
}

/**
 * @brief 保存视频帧到 SD 卡
 * 注意: 此函数在视频捕获线程中调用，需要用mutex保护以避免与停止录像的回调冲突
 */
void uvc_sd_record_save_frame(int stream_id, int media_type, unsigned char *frame_data, int frame_len, int is_keyframe)
{
    if (!g_sd_mounted)
    {
        return;
    }

#if UVC_USE_THIRD_PARTY_AVI
    /* 第三方AVI库只支持H264/H265，不支持MJPEG */
    if (media_type == DMC_MEDIA_TYPE_H264)
    {
        g_video_type = 0;  /* H264 */
    }
    else if (media_type == DMC_MEDIA_TYPE_H265)
    {
        g_video_type = 1;  /* H265 */
    }
    else
    {
        /* 不支持MJPEG，直接返回 */
        return;
    }
#endif

    /* 使用mutex保护，避免与uvc_stream_off回调冲突
     * 注意: 如果回调在中断上下文调用，这里可能会导致问题 */
    pthread_mutex_lock(&g_record_mutex);

    /* 只有在录像中才写入帧 */
    if (g_recording)
    {
        write_frame(stream_id, media_type, frame_data, frame_len, is_keyframe);
    }

    pthread_mutex_unlock(&g_record_mutex);
}

#endif /* UVC_ENABLE_SD_RECORD */

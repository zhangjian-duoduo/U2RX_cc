/**
 * @file uvc_sd_record.c
 * @brief UVC SD 卡录像功能实现
 */
#include "uvc_rtt/include/uvc_sd_record.h"
#include "uvc_rtt/include/uvc_feature_config.h"

#if UVC_ENABLE_SD_RECORD

#include <dfs.h>
#include <dfs_fs.h>

/* 根据录像格式选择不同的实现 */
#if UVC_SD_RECORD_FORMAT == 0
/* 原始流格式 - 使用 librecord */
#include "librecord.h"
#define RECORD_CONTEXT void*
static int g_record_initialized = 0;

#elif UVC_SD_RECORD_FORMAT == 1 || UVC_SD_RECORD_FORMAT == 2
/* AVI/MP4 格式 - 使用新的容器封装 */
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

    // 直接尝试挂载 SD 卡到根目录 (与 SDK 保持一致)
    ret = dfs_mount("mmcblk0p1", "/", "elm", 0, NULL);
    if (ret != 0)
    {
        printf("[SD Record] Failed to mount SD card: %d\n", ret);
        return -1;
    }

    g_sd_mounted = 1;
    printf("[SD Record] SD card mounted successfully\n");
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
    /* 原始流 - 无扩展名 */
    snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d",
             tm_now->tm_year + 1900,
             tm_now->tm_mon + 1,
             tm_now->tm_mday,
             tm_now->tm_hour,
             tm_now->tm_min,
             tm_now->tm_sec);
#elif UVC_SD_RECORD_FORMAT == 1
    /* AVI 格式 */
    snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d.avi",
             tm_now->tm_year + 1900,
             tm_now->tm_mon + 1,
             tm_now->tm_mday,
             tm_now->tm_hour,
             tm_now->tm_min,
             tm_now->tm_sec);
#elif UVC_SD_RECORD_FORMAT == 2
    /* MP4 格式 */
    snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d.mp4",
             tm_now->tm_year + 1900,
             tm_now->tm_mon + 1,
             tm_now->tm_mday,
             tm_now->tm_hour,
             tm_now->tm_min,
             tm_now->tm_sec);
#else
    /* 用户自定义格式 */
    snprintf(filename, len, "/%04d%02d%02d_%02d%02d%02d",
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
    /* 原始流格式 - 使用 librecord */
    if (!g_record_initialized)
    {
        librecord_init();
        g_record_initialized = 1;
    }
    return 0;

#elif UVC_SD_RECORD_FORMAT == 1 || UVC_SD_RECORD_FORMAT == 2
    /* AVI/MP4 格式 */
    int video_type = 0;  /* 0: H264, 1: H265 */

    g_record_ctx = uvc_record_container_init(g_record_filename,
                                             (record_container_type_t)UVC_SD_RECORD_FORMAT,
                                             g_video_width, g_video_height,
                                             g_video_fps, video_type);
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
#if UVC_SD_RECORD_FORMAT == 0
    /* 原始流格式 */
    if (g_record_initialized)
    {
        librecord_uninit();
        g_record_initialized = 0;
    }

#elif UVC_SD_RECORD_FORMAT == 1 || UVC_SD_RECORD_FORMAT == 2
    /* AVI/MP4 格式 */
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
#if UVC_SD_RECORD_FORMAT == 0
    /* 原始流格式 */
    int subtype = is_keyframe ? DMC_MEDIA_SUBTYPE_IFRAME : DMC_MEDIA_SUBTYPE_PFRAME;
    librecord_save(stream_id, media_type, subtype, frame_data, frame_len);
    return 0;

#elif UVC_SD_RECORD_FORMAT == 1 || UVC_SD_RECORD_FORMAT == 2
    /* AVI/MP4 格式 */
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

    // 初始化容器
    if (init_container() != 0)
    {
        printf("[SD Record] Failed to init container\n");
        pthread_mutex_unlock(&g_record_mutex);
        return -1;
    }

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
 */
void uvc_sd_record_save_frame(int stream_id, int media_type, unsigned char *frame_data, int frame_len, int is_keyframe)
{
    if (!g_sd_mounted)
    {
        return;
    }

    if (!g_recording)
    {
        // 自动开始录像
        printf("[SD Record] Auto start recording for media_type: %d\n", media_type);
        uvc_sd_record_start();
    }

    write_frame(stream_id, media_type, frame_data, frame_len, is_keyframe);
}

#endif /* UVC_ENABLE_SD_RECORD */

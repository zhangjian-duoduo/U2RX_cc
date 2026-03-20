/**
 * @file uvc_sd_record.h
 * @brief UVC SD 卡录像功能
 */
#ifndef __UVC_SD_RECORD_H__
#define __UVC_SD_RECORD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "uvc_feature_config.h"

#if UVC_ENABLE_SD_RECORD

/**
 * @brief 初始化 SD 卡录像模块
 * @return 0 成功, -1 失败
 */
int uvc_sd_record_init(void);

/**
 * @brief 设置视频参数 (需要在开始录像前调用)
 * @param width 视频宽度
 * @param height 视频高度
 * @param fps 帧率
 */
void uvc_sd_record_set_video_params(int width, int height, int fps);

/**
 * @brief 设置视频编码类型 (需要在开始录像前调用, 仅第三方AVI库有效)
 * @param type 0: H264, 1: H265
 */
void uvc_sd_record_set_video_type(int type);

/**
 * @brief 设置 H264/H265 SPS/PPS 数据 (需要在开始录像前调用, 仅第三方AVI库有效)
 * @param sps SPS数据指针
 * @param sps_len SPS数据长度
 * @param pps PPS数据指针
 * @param pps_len PPS数据长度
 */
void uvc_sd_record_set_sps_pps(unsigned char *sps, int sps_len, unsigned char *pps, int pps_len);

/**
 * @brief 开始录像
 * @return 0 成功, -1 失败（无 SD 卡）
 */
int uvc_sd_record_start(void);

/**
 * @brief 停止录像
 * @return 0 成功
 */
int uvc_sd_record_stop(void);

/**
 * @brief 销毁 SD 卡录像模块
 */
void uvc_sd_record_deinit(void);

/**
 * @brief 保存视频帧到 SD 卡
 * @param stream_id 流 ID
 * @param media_type 媒体类型 (H264/H265)
 * @param frame_data 帧数据
 * @param frame_len 帧长度
 * @param is_keyframe 是否为关键帧
 */
void uvc_sd_record_save_frame(int stream_id, int media_type, unsigned char *frame_data, int frame_len, int is_keyframe);

/**
 * @brief 检查 SD 卡是否已挂载
 * @return 1 已挂载, 0 未挂载
 */
int uvc_sd_record_is_mounted(void);

#endif /* UVC_ENABLE_SD_RECORD */

#endif /* __UVC_SD_RECORD_H__ */

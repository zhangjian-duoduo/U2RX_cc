/**
 * @file uvc_feature_config.h
 * @brief UVC 功能特性配置头文件
 * @description 用于控制 UVC 功能的开关，所有功能可通过修改此文件中的宏来开启或关闭
 */
#ifndef __UVC_FEATURE_CONFIG_H__
#define __UVC_FEATURE_CONFIG_H__

/**
 * @brief 中心十字线功能配置
 * @note 设置为 1 开启中心十字线功能，设置为 0 关闭
 */
#define UVC_ENABLE_CROSSHAIR       1

/**
 * @brief 十字线线宽 (像素)
 */
#define UVC_CROSSHAIR_LINE_WIDTH  4

/**
 * @brief 十字线颜色配置 (RGB + Alpha)
 */
#define UVC_CROSSHAIR_COLOR_R     255
#define UVC_CROSSHAIR_COLOR_G     0
#define UVC_CROSSHAIR_COLOR_B     0
#define UVC_CROSSHAIR_COLOR_A     255

/**
 * @brief 时间戳 OSD 显示配置
 * @note 设置为 0 关闭左上角的时间字符串 (控制 config.h 中的 FH_APP_OVERLAY_TOSD)
 *       注意: 此功能需要在 app/media_demo/config.h 中也做相应修改
 */
#define UVC_ENABLE_TOSD           0

/**
 * @brief LOGO OSD 显示配置
 * @note 设置为 1 开启 LOGO 显示，设置为 0 关闭
 */
#define UVC_ENABLE_LOGO           0

/**
 * @brief GBOX 矩形框显示配置
 * @note 设置为 1 开启 GBOX 显示，设置为 0 关闭
 */
#define UVC_ENABLE_GBOX           0

/**
 * @brief SD 卡录像功能配置
 * @note 设置为 1 开启 SD 卡录像功能，设置为 0 关闭
 *       录像文件保存在 SD 卡根目录，文件名格式: YYYYMMDD_HHMMSS.mp4
 */
#define UVC_ENABLE_SD_RECORD      1

/**
 * @brief SD 卡录像格式配置
 * @note 0: 原始 H264/H265 流 (无封装)
 *       1: AVI 封装格式
 *       2: MP4 封装格式
 *       3: 使用用户自定义库 (后续扩展)
 */
#define UVC_SD_RECORD_FORMAT     1

/**
 * @brief 使用第三方 AVI 库
 * @note 设置为 1 使用第三方 libfhavi.a 库 (需要H264/H265编码)
 *       设置为 0 使用内置简化版 AVI 代码
 */
#define UVC_USE_THIRD_PARTY_AVI  1

/**
 * @brief SD 卡挂载路径
 */
#define UVC_SD_MOUNT_PATH        "/"

/**
 * @brief 录像文件保存路径
 */
#define UVC_SD_RECORD_PATH       "/"

#endif /* __UVC_FEATURE_CONFIG_H__ */

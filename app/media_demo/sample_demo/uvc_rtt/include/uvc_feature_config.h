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

#endif /* __UVC_FEATURE_CONFIG_H__ */

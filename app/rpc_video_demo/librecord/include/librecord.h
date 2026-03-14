#ifndef __librecord_h__
#define __librecord_h__

#ifndef DMC_MEDIA_TYPE_H264
#define DMC_MEDIA_TYPE_H264 (1 << 8)   /*H264*/
#endif
#ifndef DMC_MEDIA_TYPE_H265
#define DMC_MEDIA_TYPE_H265 (1 << 9)   /*H265*/
#endif
#ifndef DMC_MEDIA_TYPE_JPEG
#define DMC_MEDIA_TYPE_JPEG (1 << 10)  /*JPEG*/
#endif
#ifndef DMC_MEDIA_TYPE_AUDIO
#define DMC_MEDIA_TYPE_AUDIO (1 << 11) /*Audio*/
#endif
#ifndef DMC_MEDIA_TYPE_MJPEG
#define DMC_MEDIA_TYPE_MJPEG (1 << 12) /*motion JPEG*/
#endif

#ifndef MAX_VPU_CHN_NUM
#define MAX_VPU_CHN_NUM 4
#endif
#ifndef MAX_GRP_NUM
#define MAX_GRP_NUM 2
#endif

#ifndef DMC_MEDIA_SUBTYPE_IFRAME
#define DMC_MEDIA_SUBTYPE_IFRAME (1 << 0)
#endif
#ifndef DMC_MEDIA_SUBTYPE_PFRAME
#define DMC_MEDIA_SUBTYPE_PFRAME (1 << 1)
#endif

#ifndef STREAM_MAGIC
#define STREAM_MAGIC (0x13769849)
#endif

int librecord_init(void);

int librecord_save(int media_chn, int media_type, int media_subtype, unsigned char *frame_data, int frame_len);

int librecord_uninit(void);

#ifdef FH_USING_AOV_DEMO
extern pthread_mutex_t g_aov_mutex;
#endif
#endif /*__librecord_h__*/

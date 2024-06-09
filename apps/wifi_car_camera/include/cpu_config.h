#ifndef CPU_CONFIG_H
#define CPU_CONFIG_H



#if SDRAM_SIZE == (64 * 1024 * 1024)

#define FAT_CACHE_NUM   64

#define VIDEO0_REC_FORMAT   VIDEO_FMT_MOV  //VIDEO_FMT_AVI
#define VIDEO1_REC_FORMAT   VIDEO_FMT_MOV  //VIDEO_FMT_AVI

#define VIDEO4_REC_FORMAT   VIDEO_FMT_MOV

#ifdef THREE_WAY_ENABLE
#define VIDEO3_REC_FORMAT   VIDEO_FMT_AVI
#define VIDEO2_REC_FORMAT   VIDEO_FMT_MOV
#define VREC0_FBUF_SIZE     (6*1024*1024)
#define VREC1_FBUF_SIZE     (6*1024*1024)
#define VREC3_FBUF_SIZE     (6*1024*1024)
#else
#define VIDEO3_REC_FORMAT   VIDEO_FMT_MOV
#define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
#define VREC0_FBUF_SIZE     (8*1024*1024)
#define VREC1_FBUF_SIZE     (8*1024*1024)
#define VREC2_FBUF_SIZE     (4*1024*1024)
#define VREC3_FBUF_SIZE     (8*1024*1024)
#endif

#define VREC4_FBUF_SIZE     (4*1024*1024)
#define AUDIO_BUF_SIZE      (128*1024)//(128*1024)

#define USB_CAMERA_BUF_SIZE (1 * 1024 * 1024) // + 512 * 1024)
#define CAMERA_CAP_BUF_SIZE (3 * 1024 * 1024)




#define NET_VREC0_FBUF_SIZE (4 * 1024 * 1024)
#define NET_AUDIO_BUF_SIZE      (128 * 1024)


#elif SDRAM_SIZE == (32 * 1024 * 1024)

#define FAT_CACHE_NUM   64

#define VIDEO0_REC_FORMAT   VIDEO_FMT_MOV
#define VIDEO1_REC_FORMAT   VIDEO_FMT_MOV
#define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
#define VIDEO3_REC_FORMAT   VIDEO_FMT_MOV


// 如果CPU使用571x系列，根据ddr大小各buf做相应裁剪
#define VREC0_FBUF_SIZE     (2*1024*1024)
#define VREC1_FBUF_SIZE     (1*1024*1024)
#define VREC2_FBUF_SIZE     (2*1024*1024)
#define VREC3_FBUF_SIZE     (1*1024*1024 + 512*1024)
// #define VREC4_FBUF_SIZE     (0*1024*1024)
#define AUDIO_BUF_SIZE      (128*1024)//(300*1024)

#define USB_CAMERA_BUF_SIZE (1 * 1024 * 1024) // + 512 * 1024)
#define CAMERA_CAP_BUF_SIZE (1 * 1024 * 1024)

#define NET_VREC0_FBUF_SIZE (1 * 1024 * 1024)
#define NET_AUDIO_BUF_SIZE      (128 * 1024)




#elif SDRAM_SIZE == (16 * 1024 * 1024)

#define FAT_CACHE_NUM   64

#define VIDEO0_REC_FORMAT   VIDEO_FMT_MOV
#define VIDEO1_REC_FORMAT   VIDEO_FMT_MOV
#define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
#define VIDEO3_REC_FORMAT   VIDEO_FMT_MOV


// 如果CPU使用571x系列，根据ddr大小各buf做相应裁剪
#define VREC0_FBUF_SIZE     (1*1024*1024)
#define VREC1_FBUF_SIZE     (0*1024*1024)
#define VREC2_FBUF_SIZE     (0*1024*1024)
#define VREC3_FBUF_SIZE     (1*1024*1024 + 512*1024)
// #define VREC4_FBUF_SIZE     (0*1024*1024)
#define AUDIO_BUF_SIZE      (128*1024)//(300*1024)

#define USB_CAMERA_BUF_SIZE (1 * 1024 * 1024) // + 512 * 1024)
#define CAMERA_CAP_BUF_SIZE (2 * 1024 * 1024)

#define NET_VREC0_FBUF_SIZE (1 * 1024 * 1024)
#define NET_AUDIO_BUF_SIZE      (128 * 1024)
#endif























#endif

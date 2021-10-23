#ifndef _YUYV_TO_JEPG_H_
#define _YUYV_TO_JEPG_H_

int compress_yuyv_to_jpeg(unsigned char *buf, unsigned char *buffer, int size, int quality);

void yuv_to_rgb(unsigned char *yuv, unsigned char *rgb);

long rgb_to_jpeg(unsigned char *rgb, unsigned char *jpeg);

#endif

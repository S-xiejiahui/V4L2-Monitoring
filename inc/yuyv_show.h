#ifndef _YUYV_SHOW_
#define _YUYV_SHOW_

void process_yuv_image(unsigned char *yuv, int width, int height);

void yuv_to_rgb(unsigned char *yuv, unsigned char *rgb);

int show_video_data(char *pjpg_buf, unsigned int jpg_buf_size, int point_x, int point_y, int numerator, int denominator);

void jpeg_display(char *jpeg_pathname, unsigned int start_x, unsigned int start_y, int numerator, int denominator);


#endif

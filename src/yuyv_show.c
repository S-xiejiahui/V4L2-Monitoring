#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jpeglib.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include "lcd.h"

#define IMAGE_W 640
#define IMAGE_H 480

int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
	 unsigned int pixel32 = 0;
	 unsigned char pixel[3];// = (unsigned char *)&pixel32;
	 int r, g, b;
	 r = y + (1.370705 * (v-128));
	 g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
	 b = y + (1.732446 * (u-128));
	 if(r > 255) r = 255;
	 if(g > 255) g = 255;
	 if(b > 255) b = 255;
	 if(r < 0) r = 0;
	 if(g < 0) g = 0;
	 if(b < 0) b = 0;
	 pixel[0] = r * 220 / 256;
	 pixel[1] = g * 220 / 256;
	 pixel[2] = b * 220 / 256;

	 pixel32 = (pixel[0] << 16 ) | (pixel[1] << 8) | (pixel[2]);
	 return pixel32;
}
/*
	函数功能：将 yuyv格式 转化为 argb格式 
	参数： yuv 即存储yuyv格式图像数据的数组 这个数组 是一个 unsigned char * 即映射区域的首地址 
			映射首地址存储在 ub[] 中成员 start 中 
			width 视频图像的宽度 
			height 视频图像的高度 
	返回值： 无
*/
void process_yuv_image(unsigned char *yuv, int width, int height)
{
	unsigned int in, out = 0;
	unsigned int pixel_16;
	unsigned char pixel_24[3];
	unsigned int pixel32;
	 int y0, u, y1, v;

	int pixel_num = 0;

 	for(in = 0; in < width * height * 2; in += 4) // 640 *480 *2 就是数组的总大小  每4个一组
	{
		//四字节组合成一个int型数据 
		pixel_16 =
			yuv[in + 3] << 24 |
			yuv[in + 2] << 16 |
			yuv[in + 1] <<  8 |
			yuv[in + 0];
		//将几个数据分量提取出来  y0在最低字节 u在 次低字节 y1在次高字节 V在 高字节 
		y0 = (pixel_16 & 0x000000ff);
		u  = (pixel_16 & 0x0000ff00) >>  8;
		y1 = (pixel_16 & 0x00ff0000) >> 16;
		v  = (pixel_16 & 0xff000000) >> 24;
		
		//通过y0 u v 计算出颜色数据  这里算的是第一个点 
 	 	pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
		pixel_num ++;
		//画点函数 
		lcd_draw_point(pixel_num/width,pixel_num%width,pixel32);

		//这里算的是第二个点 
		pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
		pixel_num ++;
		lcd_draw_point(pixel_num/width,pixel_num%width,pixel32);
 	}
	return ;
}


/*****************************************************************************************
 								将JPEG图像数据显示函数
*****************************************************************************************/
int show_video_data(char *pjpg_buf, unsigned int jpg_buf_size, int point_x, int point_y, int numerator, int denominator)  
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr 		  jerr;	
	char *pjpg = NULL;
	unsigned int i = 0;
	unsigned int color = 0;
	
	pjpg = pjpg_buf;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	
	jpeg_mem_src(&cinfo,pjpg,jpg_buf_size);
	jpeg_read_header(&cinfo, TRUE);
	//放大、缩小倍数
	cinfo.scale_num   = numerator;//分子
    cinfo.scale_denom = denominator;//分母

	jpeg_start_decompress(&cinfo);	
	
	unsigned char *buffer = malloc(cinfo.output_width * cinfo.output_components);

//６. 读取一行或者多行扫描线数据并处理
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, &buffer, 1); //dinfo.output_scanline + 1
		//deal with scanlines . RGB/ARGB
		int x; //一行的像素点数量

		char *p = buffer;
		for (x = 0; x < cinfo.output_width; x++)
		{
			unsigned char r, g, b, a = 0;
			int color;
			if (cinfo.output_components == 3)
			{
				r = *p++;
				g = *p++;
				b = *p++;
			} else if (cinfo.output_components == 4)
			{
				a = *p++;
				r = *p++;
				g = *p++;
				b = *p++;
			}
			color = (a << 24) | (r << 16) | (g << 8) |(b) ;

			lcd_draw_point(cinfo.output_scanline - 1 + point_y, x + point_x,  color); 	
		}
	}
//7. 调用　jpeg_finish_decompress()完成解压过程
	jpeg_finish_decompress(&cinfo);

//８. 调用jpeg_destroy_decompress()释放jpeg解压对象dinfo
	jpeg_destroy_decompress(&cinfo);
	free(buffer);
	return 0;
}

/*****************************************************************************************
 								显示JPEG图片
*****************************************************************************************/
void jpeg_display(char *jpeg_pathname, unsigned int start_x, unsigned int start_y, int numerator, int denominator)
{
//1. 分配并初始化一个jpeg解压对象

	struct jpeg_decompress_struct dinfo; //定义了一个jpeg的解压对象
	struct jpeg_error_mgr jerr; //定义一个错误变量
	dinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&dinfo); //初始化这个解压对象
	
//2. 指定要解压缩的图像文件

//方法一：
	FILE *infile;
	infile = fopen(jpeg_pathname, "r");
	if (infile == NULL)
	{
		fprintf(stderr, "[%d][%s]fopen error", __LINE__, __FUNCTION__);
		return;
	}
	jpeg_stdio_src(&dinfo, infile); //为解压对象dinfo指定要解压的文件
	jpeg_read_header(&dinfo, TRUE);//

    dinfo.scale_num   = numerator;
    dinfo.scale_denom = denominator;

	jpeg_start_decompress(&dinfo); 

	unsigned char *buffer = malloc(dinfo.output_width * dinfo.output_components);

	while (dinfo.output_scanline < dinfo.output_height)
	{
		jpeg_read_scanlines(&dinfo, &buffer, 1); //dinfo.output_scanline + 1

		int x; //一行的像素点数量
		char *p = buffer;
		for (x = 0; x < dinfo.output_width; x++)
		{
			unsigned char r, g, b, a = 0;
			int color;
			if (dinfo.output_components == 3)
			{
				r = *p++;
				g = *p++;
				b = *p++;
			} else if (dinfo.output_components == 4)
			{
				a = *p++;
				r = *p++;
				g = *p++;
				b = *p++;
			}
			color = (a << 24) | (r << 16) | (g << 8) |(b) ;

			lcd_draw_point(dinfo.output_scanline - 1 + start_y, x + start_x,  color); 	
		}
	}
//7. 调用　jpeg_finish_decompress()完成解压过程

	jpeg_finish_decompress(&dinfo);

//８. 调用jpeg_destroy_decompress()释放jpeg解压对象dinfo

	jpeg_destroy_decompress(&dinfo);
	free(buffer);
	fclose(infile);
	return;
}

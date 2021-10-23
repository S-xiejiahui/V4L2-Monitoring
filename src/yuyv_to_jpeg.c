#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>
#include "jpeglib.h"

 
#define IMAGE_W 640
#define IMAGE_H 480
 
#define OUTPUT_BUF_SIZE 4096
 
typedef struct {
    struct jpeg_destination_mgr pub;
    JOCTET * buffer;
    unsigned char *outbuffer;
    int outbuffer_size;
    unsigned char *outbuffer_cursor;
    int *written;
}mjpg_destination_mgr;
 
typedef mjpg_destination_mgr *mjpg_dest_ptr;
 
METHODDEF(void) init_destination(j_compress_ptr cinfo) {
    mjpg_dest_ptr dest =(mjpg_dest_ptr) cinfo->dest;
    dest->buffer =(JOCTET *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_IMAGE, OUTPUT_BUF_SIZE*sizeof(JOCTET));
    *(dest->written) = 0;
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}
 
METHODDEF(boolean) empty_output_buffer(j_compress_ptr cinfo) {
    mjpg_dest_ptr dest =(mjpg_dest_ptr) cinfo->dest;
    memcpy(dest->outbuffer_cursor, dest->buffer, OUTPUT_BUF_SIZE);
    dest->outbuffer_cursor += OUTPUT_BUF_SIZE;
    *(dest->written) += OUTPUT_BUF_SIZE;
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
    return TRUE;
}
 
METHODDEF(void) term_destination(j_compress_ptr cinfo) {
    mjpg_dest_ptr dest =(mjpg_dest_ptr) cinfo->dest;
    size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
    /* Write any data remaining in the buffer */
    memcpy(dest->outbuffer_cursor, dest->buffer, datacount);
    dest->outbuffer_cursor += datacount;
    *(dest->written) += datacount;
}
 
void dest_buffer(j_compress_ptr cinfo, unsigned char *buffer, int size, int *written) {
    mjpg_dest_ptr dest;
    if(cinfo->dest == NULL) {
        cinfo->dest =(struct jpeg_destination_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(mjpg_destination_mgr));
    }
 
    dest =(mjpg_dest_ptr)cinfo->dest;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;
    dest->outbuffer = buffer;
    dest->outbuffer_size = size;
    dest->outbuffer_cursor = buffer;
    dest->written = written;
}
 
//......YUYV.....JPEG..
int compress_yuyv_to_jpeg(unsigned char *buf, unsigned char *buffer, int size, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    unsigned char *line_buffer, *yuyv;
    int z;
    static int written;
    //int count = 0;
    //printf("%s\n", buf);
    line_buffer = (unsigned char*)calloc(IMAGE_W * 3, 1);
    yuyv = buf;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    /* jpeg_stdio_dest(&cinfo, file); */
    dest_buffer(&cinfo, buffer, size, &written);
    cinfo.image_width = IMAGE_W;
    cinfo.image_height = IMAGE_H;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    z = 0;
    while(cinfo.next_scanline < IMAGE_H) {
        int x;
        unsigned char *ptr = line_buffer;
        for(x = 0; x < IMAGE_W; x++) {
            int r, g, b;
            int y, u, v;
            if(!z)
                y = yuyv[0] << 8;
            else
                y = yuyv[2] << 8;
            u = yuyv[1] - 128;
            v = yuyv[3] - 128;
            r =(y +(359 * v))>> 8;
            g =(y -(88 * u) -(183 * v)) >> 8;
            b =(y +(454 * u)) >> 8;
            *(ptr++) =(r > 255) ? 255 :((r < 0) ? 0 : r);
            *(ptr++) =(g > 255) ? 255 :((g < 0) ? 0 : g);
            *(ptr++) =(b > 255) ? 255 :((b < 0) ? 0 : b);
            if(z++) {
                z = 0;
                yuyv += 4;
            }
        }
        row_pointer[0] = line_buffer;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    free(line_buffer);
    return(written);
}
/*
    使用方法：
    compress_yuyv_to_jpeg(buf, buffer,(IMAGE_W * IMAGE_H), 80);
*/

void yuv_to_rgb(unsigned char *yuv, unsigned char *rgb)
{
    unsigned int i;
    unsigned char* y0 = yuv + 0;
    unsigned char* u0 = yuv + 1;
    unsigned char* y1 = yuv + 2;
    unsigned char* v0 = yuv + 3;
 
    unsigned  char* r0 = rgb + 0;
    unsigned  char* g0 = rgb + 1;
    unsigned  char* b0 = rgb + 2;
    unsigned  char* r1 = rgb + 3;
    unsigned  char* g1 = rgb + 4;
    unsigned  char* b1 = rgb + 5;
 
    float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;
 
    for(i = 0; i <= (IMAGE_W * IMAGE_H) / 2 ;i++)
    {
        bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128);
        gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);
        rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128);
 
        bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128);
        gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);
        rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128);
 
 
        if(rt0 > 250)      rt0 = 255;
        if(rt0< 0)        rt0 = 0;
 
        if(gt0 > 250)     gt0 = 255;
        if(gt0 < 0)    gt0 = 0;
 
        if(bt0 > 250)    bt0 = 255;
        if(bt0 < 0)    bt0 = 0;
 
        if(rt1 > 250)    rt1 = 255;
        if(rt1 < 0)    rt1 = 0;
 
        if(gt1 > 250)    gt1 = 255;
        if(gt1 < 0)    gt1 = 0;
 
        if(bt1 > 250)    bt1 = 255;
        if(bt1 < 0)    bt1 = 0;
 
        *r0 = (unsigned char)rt0;
        *g0 = (unsigned char)gt0;
        *b0 = (unsigned char)bt0;
 
        *r1 = (unsigned char)rt1;
        *g1 = (unsigned char)gt1;
        *b1 = (unsigned char)bt1;
 
        yuv = yuv + 4;
        rgb = rgb + 6;
        if(yuv == NULL)
            break;
 
        y0 = yuv;
        u0 = yuv + 1;
        y1 = yuv + 2;
        v0 = yuv + 3;
 
        r0 = rgb + 0;
        g0 = rgb + 1;
        b0 = rgb + 2;
        r1 = rgb + 3;
        g1 = rgb + 4;
        b1 = rgb + 5;
    }
}

long rgb_to_jpeg(unsigned char *rgb, unsigned char *jpeg)
{
    long unsigned int jpeg_size;
    struct jpeg_compress_struct jcs;
    struct jpeg_error_mgr jem;
    JSAMPROW row_pointer[1];
    int row_stride;
 
    jcs.err = jpeg_std_error(&jem);
    jpeg_create_compress(&jcs);
 
    jpeg_mem_dest(&jcs, &jpeg, &jpeg_size);//就是这个函数！！！！！！！
 
    jcs.image_width = IMAGE_W;
    jcs.image_height = IMAGE_H;
 
    jcs.input_components = 3;//1;
    jcs.in_color_space = JCS_RGB;//JCS_GRAYSCALE;
 
    jpeg_set_defaults(&jcs);
    jpeg_set_quality(&jcs, 180, TRUE);
 
    jpeg_start_compress(&jcs, TRUE);
    row_stride =jcs.image_width * 3;
 
    while(jcs.next_scanline < jcs.image_height){//对每一行进行压缩
        row_pointer[0] = &rgb[jcs.next_scanline * row_stride];
        (void)jpeg_write_scanlines(&jcs, row_pointer, 1);
    }
    jpeg_finish_compress(&jcs);
    jpeg_destroy_compress(&jcs);
 
#ifdef JPEG    //jpeg 保存，测试用
    FILE *jpeg_fd;
 
    jpeg_fd = fopen("./jpeg.jpg","w");
    if(jpeg_fd < 0 ){
        perror("open jpeg.jpg failed!\n");
        exit(-1);
    }
 
    fwrite(jpeg, jpeg_size, 1, jpeg_fd);
    fclose(jpeg_fd);
#endif
    return jpeg_size;
}
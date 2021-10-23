#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "queue.h"
#include "main.h"
#include "yuyv_show.h"

/**********************************************************
 * @brief: 
 * @note:
 * @return;
 *********************************************************/
struct usebuf
{
	void *start; 					//映射区域的起点地址 
	int length; 					//映射区域的大小
};

static int fd = -1;
#define FRAME_RATE 6				//帧率
struct usebuf ub[FRAME_RATE] = {0}; //申请 MAX_PICTURE个 缓冲区 
pthread_mutex_t mutex;
/**********************************************************
 * @brief: 打开摄像头
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_turn_on_the_camera(char *device_name)
{
	fd = open(device_name, O_RDWR);
	if (fd == -1)
	{
		printf("Waiting for the camera to connect...\n");
		return -1;
	}
	return fd;
}
/**********************************************************
 * @brief: 获取设备信息
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_query_device_function(void)
{
	struct v4l2_capability cap;
	int ret = ioctl(fd,VIDIOC_QUERYCAP,&cap); 
	if(ret < 0)
	{
		perror("ioctl VIDIOC_QUERYCAP error");
		return -1;
	}
	printf("cap.capabilities = %d",cap.capabilities);
	return 0;
}
/**********************************************************
 * @brief: 获取摄像头支持格式
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_show_Supported_video_formats(void)
{  
	struct v4l2_fmtdesc check;
	check.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;		//视频录制
	check.index = 0;
	
	int ret = ioctl(fd,VIDIOC_ENUM_FMT,&check);
	if(ret < 0)
	{
		perror("ioctl VIDIOC_ENUM_FMT error");
		return -1;
	}
	printf(" format:%s\n",check.description); 		//设置了视频录制之后，看一下是什么视频格式,根据输出结构可以知道 视频格式为 YUYV 
}
/**********************************************************
 * @brief: 设置摄像头视频格式
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_set_video_format(void)
{
	struct v4l2_format s_fmt;
	s_fmt.fmt.pix.width = 640; 						//因为这个摄像头的大小就是 640 * 480 的 
	s_fmt.fmt.pix.height = 480; 
	s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;	//即格式设置为YUYV格式
	s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;		//视频录制
	
	int ret = ioctl(fd,VIDIOC_S_FMT,&s_fmt);
	if(ret < 0)
	{
		perror("ioctl VIDIOC_S_FMT error");
		return -1;
	}
	printf("pix.field = %d\n",s_fmt.fmt.pix.field);	//查看视频属性

	struct v4l2_requestbuffers reqbuf;
	memset(&reqbuf,0,sizeof(reqbuf));				//清空reqbuf
	
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.count = FRAME_RATE; 						//每帧图像数据 最大是 10  1s钟最多获取30张图片 
	reqbuf.memory = V4L2_MEMORY_MMAP;
	
	ret = ioctl(fd,VIDIOC_REQBUFS,&reqbuf);
	if(ret < 0)
	{
		perror("ioctl VIDIOC_REQBUFS error");
		return -1;
	}
	else 
	{
		printf("request buf success\n");
	}
	return 0;
}
/**********************************************************
 * @brief: 申请摄像头映射内存
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_request_video_buffer(void)
{
	struct v4l2_buffer buf;
	int i = 0;
	for(i=0;i<FRAME_RATE;i++)
	{
		memset(&buf,0,sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;		//视频录制
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i; 								//i 就是一帧数据中的第几张图片 
		int ret = ioctl(fd,VIDIOC_QUERYBUF,&buf);
		if(ret <0)
		{
			perror("ioctl VIDIOC_QUERYBUF error");
			return -1;
		}
		else 
		{
			printf("ioctl VIDIOC_QUERYBUF success\n");
		}
		printf("buf.index = %d,buf.length=%d\n",buf.index,buf.length);
		
		ub[buf.index].length = buf.length;			//每查到一个内核缓冲 就映射一次
		ub[buf.index].start = mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
		if(ub[buf.index].start == NULL)
		{
			perror("mmap ub[buf.index] error\n");
			return -1;
		}
		else 
		{
			printf("ub[%d].length = %d\n",i,ub[i].length);
		}
		
		ret=ioctl(fd,VIDIOC_QBUF,&buf);				//将映射的内存缓冲区 添加到采集队列中
		if(ret <0)
		{
			perror("ioctl VIDIOC_QBUF error");
			return -1;
		}
		else 
		{
			printf("ioctl VIDIOC_QBUF success\n");
		}
	}
	return 0;
}
/**********************************************************
 * @brief: 开始视频采集
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_start_collecting(Queue *queue_display)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;				//视频录制
	int ret = ioctl(fd,VIDIOC_STREAMON,&type);
	if(ret < 0)
	{
		perror("ioctl VIDIOC_STREAMON error");
		return -1;
	}
	else 
	{
		printf("ioctl VIDIOC_STREAMON success\n");
	}
	
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds); 								//将fd加入到了 文件描述符集中 
	
	while(1)
	{
		int r = select(fd+1,&rfds,NULL,NULL,NULL);
		if(r<=0)
			continue;								//失败或者是 超时 再次监听 
		else  										//监听到了 
		{
			if(FD_ISSET(fd,&rfds)) 					//操作  
			{
				struct v4l2_buffer  vbuf;			//获取一帧数据到视频数据缓冲区中 即 buf中 
				memset(&vbuf,0,sizeof(vbuf));
				vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vbuf.memory = V4L2_MEMORY_MMAP;
		
				ret = ioctl(fd,VIDIOC_DQBUF,&vbuf);
				if(ret < 0)
				{
					perror("ioctl VIDIOC_DQBUF error");
					return -1;
				}
				else 
				{
					//printf("ioctl VIDIOC_DQBUF success\n");
				}
				//上锁
				pthread_mutex_lock(&mutex);
				push_queue(queue_display, ub[vbuf.index].start);
				pthread_mutex_unlock(&mutex);
				
				//预留在这个位置需要 将视频显示在 lcd屏幕上  xxxx  关键任务的核心 在这个位置 即采集到的数据如何取显示 
				//process_yuv_image(ub[vbuf.index].start,640,480); // buf就是缓冲区 缓冲区是哪一个 就是你申请时给的编号
				//显示结束后 需要将这个缓冲区再次放到任务队列中继续采集  
				ret=ioctl(fd,VIDIOC_QBUF,&vbuf);
				if(ret <0)
				{
					perror("ioctl VIDIOC_QBUF error");
					//return -1;
				}
				else 
				{
					//printf("ioctl VIDIOC_QBUF success\n");
				}
			}
		}
	}
}
/**********************************************************
 * @brief: 结束视频采集
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_end_of_acquisition(void)
{
	enum v4l2_buf_type type;  
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
  
    if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))  
    {  
        perror("end_of_acquisition failed:"); 
        return -1;  
    }
	close(fd);
    return 0;
}
/**********************************************************
 * @brief: 回收映射内存
 * @note:
 * @return;
 *********************************************************/
extern int v4l2_resource_recovery(void)
{
	int i = 0;
	for(i = 0;i< FRAME_RATE;i++)
	{
		munmap(ub[i].start,ub[i].length);
	}
}


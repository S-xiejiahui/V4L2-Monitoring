#ifndef _MAIN_H_
#define _MAIN_H_

/*打开摄像头*/
extern int v4l2_turn_on_the_camera(char *device_name);
/**获取设备信息*/
extern int v4l2_query_device_function(void);
/*获取摄像头支持格式*/
extern int v4l2_show_Supported_video_formats(void);
/*设置摄像头视频格式*/
extern int v4l2_set_video_format(void);
/*申请摄像头映射内存*/
extern int v4l2_request_video_buffer(void);
/*开始视频采集*/
extern int v4l2_start_collecting(Queue *queue_display);
/*结束视频采集*/
extern int v4l2_end_of_acquisition(void);
/*回收映射内存*/
extern int v4l2_resource_recovery(void);

#endif

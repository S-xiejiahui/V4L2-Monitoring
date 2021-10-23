#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

static int lcd_fd = -1; //屏幕文件描述符 
static int *plcd = NULL; //映射首地址 
static struct fb_var_screeninfo lcd_info;

/***************************************************************
		Initialization screen
		@parameter:	none
		@return : success return 0
				: failed return -1
***************************************************************/
int lcd_init(void)
{
	//第一步：打开屏幕 
	lcd_fd = open("/dev/fb0",O_RDWR);
	if(lcd_fd < 0)
	{
		perror("open lcd error");
		return -1;
	}

	//第二步：获取屏幕属性 
	
	int ret = ioctl(lcd_fd,FBIOGET_VSCREENINFO,&lcd_info);
	if(ret < 0)
	{
		perror("ioctl lcd failed");
		return -1;
	}

	printf("lenth = %d,high = %d\n",lcd_info.xres,lcd_info.yres);
	printf("color depth = %d\n",lcd_info.bits_per_pixel);
	printf("red.offset=%d,red.length=%d,red.msb_right = %d\n",lcd_info.red.offset,
			lcd_info.red.length,lcd_info.red.msb_right);
	printf("green.offset=%d,green.length=%d,green.msb_right = %d\n",lcd_info.green.offset,
			lcd_info.green.length,lcd_info.green.msb_right);
	printf("blue.offset=%d,blue.length=%d,blue.msb_right = %d\n",lcd_info.blue.offset,
			lcd_info.blue.length,lcd_info.blue.msb_right);
			
	//第三步：映射屏幕
	plcd = (int *)mmap(NULL,lcd_info.xres*lcd_info.yres*lcd_info.bits_per_pixel/8,
	PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
	if(plcd == NULL)
	{
		perror("mmap failed:");
		return -1;
	}
	return 0;
}
/***************************************************************
		uninit screen
		@parameter:	none
		@return : success return 0
				: failed return -1
***************************************************************/
int lcd_uninit(void)
{
	//解除映射 
	int ret = munmap(plcd,lcd_info.xres*lcd_info.yres*lcd_info.bits_per_pixel/8);
	if(ret < 0)
	{
		perror("munmap failed:");
		return -1;
	}
	//关闭屏幕
	ret = close(lcd_fd);
	if(ret < 0)
	{
		perror("close lcd failed:");
		return -1;
	}
	return 0;
}
/***************************************************************
		Screen painting point
		@parameter:	none
		@return : none
***************************************************************/
void lcd_draw_point(int x,int y,int color)
{
	if(x>=0 && x <480 && y>=0 && y<800)
		*(plcd + x*800 +y) = color;
}
/***************************************************************
		Screen painting word
		@parameter:	
			@name: word
			@x0: start coordinates of x
			@y0: ending coordinates of y
			@len: size of word
			@color：show color
		@return : none
***************************************************************/

void draw_word(unsigned char *name, int x0,int y0, int len, int size,int color)
{
	int flag = len/8; //即 字的宽度使用几个字节的数据表示 
	
	int i,j;
	
	for(i=0;i<size;i++) //遍历每一个字节 
	{
		for(j=0;j<8;j++) //遍历每一个字节的每一位 
		{
			if(name[i]>>j & 0x01)  //为真表示这一位为1 			
			{
				lcd_draw_point(x0+i/flag,y0+(i%flag)*8+(7-j),color);
			}
		}
	}
}


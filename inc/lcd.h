#ifndef _lcd_h_
#define _lcd_h_
/***************************************************************
		Initialization screen
		@parameter:	none
		@return : success return 0
				: failed return -1
***************************************************************/
int lcd_init(void);
/***************************************************************
		uninit screen
		@parameter:	none
		@return : success return 0
				: failed return -1
***************************************************************/
int lcd_uninit(void);
/***************************************************************
		Screen painting point
		@parameter:	none
		@return : none
***************************************************************/
void lcd_draw_point(int x,int y,int color);
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
void draw_word(unsigned char *name, int x0,int y0, int len, int size,int color);


#endif 

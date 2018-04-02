/*
Simple dingux game framework license

Copyright © 2015–2018, Popov Evgeniy Alekseyevich

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Third–party code license

Pixel packing algorithm bases on code from SVGALib. SVGALib is public domain.
SVGALib homepage: http://www.svgalib.org/
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <linux/fb.h>

#define SDGF_KEY_NONE 0
#define SDGF_KEY_UP KEY_UP
#define SDGF_KEY_DOWN KEY_DOWN
#define SDGF_KEY_LEFT KEY_LEFT
#define SDGF_KEY_RIGHT KEY_RIGHT
#define SDGF_KEY_A KEY_LEFTCTRL
#define SDGF_KEY_B KEY_LEFTALT
#define SDGF_KEY_X KEY_SPACE
#define SDGF_KEY_Y KEY_LEFTSHIFT
#define SDGF_KEY_L KEY_TAB
#define SDGF_KEY_R KEY_BACKSPACE
#define SDGF_KEY_START KEY_ENTER
#define SDGF_KEY_SELECT KEY_ESC
#define SDGF_KEY_POWER KEY_POWER
#define SDGF_KEY_HOLD KEY_HOLD
#define SDGF_GAMEPAD_HOLDING 2
#define SDGF_GAMEPAD_PRESS 1
#define SDGF_GAMEPAD_RELEASE 0

struct SDGF_Color
{
 unsigned char blue:8;
 unsigned char green:8;
 unsigned char red:8;
};

struct SDGF_Key
{
 unsigned short int button;
 unsigned char state;
};

struct TGA_head
{
 unsigned char id:8;
 unsigned char color_map:8;
 unsigned char type:8;
};

struct TGA_map
{
 unsigned short int index:16;
 unsigned short int length:16;
 unsigned char map_size:8;
};

struct TGA_image
{
 unsigned short int x:16;
 unsigned short int y:16;
 unsigned short int width:16;
 unsigned short int height:16;
 unsigned char color:8;
 unsigned char alpha:3;
 unsigned char direction:5;
};

struct PCX_head
{
 unsigned char vendor:8;
 unsigned char version:8;
 unsigned char compress:8;
 unsigned char color:8;
 unsigned short int min_x:16;
 unsigned short int min_y:16;
 unsigned short int max_x:16;
 unsigned short int max_y:16;
 unsigned short int vertical_dpi:16;
 unsigned short int horizontal_dpi:16;
 unsigned char palette[48];
 unsigned char reversed:8;
 unsigned char planes:8;
 unsigned short int plane_length:16;
 unsigned short int palette_type:16;
 unsigned short int screen_width:16;
 unsigned short int screen_height:16;
 unsigned char filled[54];
};

struct SDGF_Box
{
 unsigned long int x:32;
 unsigned long int y:32;
 unsigned long int width:32;
 unsigned long int height:32;
};

class SDGF_Frame
{
 protected:
 unsigned long int width;
 unsigned long int height;
 unsigned long int length;
 unsigned short int *buffer;
 unsigned short int get_bgr565(const unsigned char red,const unsigned char green,const unsigned char blue);
 public:
 SDGF_Frame();
 ~SDGF_Frame();
 void draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned char red,const unsigned char green,const unsigned char blue);
 void clear_screen();
 unsigned long int get_width();
 unsigned long int get_height();
};

class SDGF_Screen:public SDGF_Frame
{
 private:
 int device;
 unsigned long int start;
 unsigned char *primary;
 fb_fix_screeninfo configuration;
 fb_var_screeninfo setting;
 public:
 SDGF_Screen();
 ~SDGF_Screen();
 void refresh();
 SDGF_Screen* get_handle();
};

class SDGF_Gamepad
{
 private:
 int device;
 long int length;
 input_event input;
 SDGF_Key key;
 public:
 SDGF_Gamepad();
 ~SDGF_Gamepad();
 void initialize();
 void update();
 unsigned short int get_hold();
 unsigned short int get_press();
 unsigned short int get_release();
};

class SDGF_Memory
{
 public:
 unsigned long int get_total_memory();
 unsigned long int get_free_memory();
};

class SDGF_System
{
 public:
 SDGF_System();
 ~SDGF_System();
 unsigned long int get_random(const unsigned long int number);
 void quit();
 void run(const char *command);
 char* read_environment(const char *variable);
 void enable_logging(const char *name);
};

class SDGF_Timer
{
 private:
 unsigned long int interval;
 time_t start;
 public:
 SDGF_Timer();
 ~SDGF_Timer();
 void set_timer(const unsigned long int seconds);
 bool check_timer();
};

class SDGF_Primitive
{
 private:
 SDGF_Screen *surface;
 public:
 SDGF_Primitive();
 ~SDGF_Primitive();
 void initialize(SDGF_Screen *Screen);
 void draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2,const unsigned char red,const unsigned char green,const unsigned char blue);
 void draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height,const unsigned char red,const unsigned char green,const unsigned char blue);
 void draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height,const unsigned char red,const unsigned char green,const unsigned char blue);
};

class SDGF_Image
{
 private:
 unsigned long int width;
 unsigned long int height;
 unsigned char *data;
 unsigned char *create_buffer(const size_t length);
 void clear_buffer();
 FILE *open_image(const char *name);
 unsigned long int get_file_size(FILE *target);
 public:
 SDGF_Image();
 ~SDGF_Image();
 void load_tga(const char *name);
 void load_pcx(const char *name);
 unsigned long int get_width();
 unsigned long int get_height();
 size_t get_data_length();
 unsigned char *get_data();
 void destroy_image();
};

class SDGF_Canvas
{
 protected:
 unsigned long int width;
 unsigned long int height;
 unsigned long int frames;
 SDGF_Screen *surface;
 SDGF_Color *image;
 SDGF_Color *create_buffer(const unsigned long int image_width,const unsigned long int image_height);
 void draw_image_pixel(size_t offset,const unsigned long int x,const unsigned long int y);
 size_t get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y);
 private:
 void clear_buffer();
 void check_size();
 public:
 SDGF_Canvas();
 ~SDGF_Canvas();
 SDGF_Color *get_image();
 unsigned long int get_width();
 unsigned long int get_height();
 void set_frames(const unsigned long int amount);
 unsigned long int get_frames();
 void initialize(SDGF_Screen *Screen);
 void load_image(SDGF_Image &buffer);
 void mirror_image(const unsigned char kind);
 void resize_image(const unsigned long int new_width,const unsigned long int new_height);
};

class SDGF_Background:public SDGF_Canvas
{
 public:
 void draw_horizontal_background(const unsigned long int frame);
 void draw_vertical_background(const unsigned long int frame);
 void draw_background();
};

class SDGF_Sprite:public SDGF_Canvas
{
 private:
 unsigned long int current_x;
 unsigned long int current_y;
 bool compare_pixels(const SDGF_Color &first,const SDGF_Color &second);
 void draw_sprite_pixel(size_t offset,const unsigned long int x,const unsigned long int y);
 public:
 SDGF_Sprite();
 ~SDGF_Sprite();
 void clone(SDGF_Sprite &target);
 void draw_sprite_frame(const unsigned long int x,const unsigned long int y,const unsigned long int frame);
 void draw_sprite(const unsigned long int x,const unsigned long int y);
 unsigned long int get_x();
 unsigned long int get_y();
 unsigned long int get_sprite_width();
 unsigned long int get_sprite_height();
 SDGF_Sprite* get_handle();
 SDGF_Box get_box();
};

class SDGF_Text
{
 private:
 unsigned long int current_x;
 unsigned long int current_y;
 SDGF_Sprite *sprite;
 public:
 SDGF_Text();
 ~SDGF_Text();
 void set_position(const unsigned long int x,const unsigned long int y);
 void load_font(SDGF_Sprite *font);
 void draw_text(const char *text);
};

class SDGF_Collision
{
 public:
 bool check_horizontal_collision(const SDGF_Box &first,const SDGF_Box &second);
 bool check_vertical_collision(const SDGF_Box &first,const SDGF_Box &second);
 bool check_collision(const SDGF_Box &first,const SDGF_Box &second);
};
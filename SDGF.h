/*
Simple dingux game framework license

Copyright (C) 2015-2018 Popov Evgeniy Alekseyevich

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

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

enum SDGF_MIRROR_TYPE {SDGF_MIRROR_HORIZONTAL=0,SDGF_MIRROR_VERTICAL=1};
enum SDGF_BACKGROUND_TYPE {SDGF_NORMAL_BACKGROUND=0,SDGF_HORIZONTAL_BACKGROUND=1,SDGF_VERTICAL_BACKGROUND=2};
enum SDGF_SPRITE_TYPE {SDGF_SINGE_SPRITE=0,SDGF_HORIZONTAL_STRIP=1,SDGF_VERTICAL_STRIP=2};

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

void SDGF_Show_Error(const char *message);

class SDGF_Frame
{
 private:
 unsigned long int width;
 unsigned long int height;
 size_t length;
 unsigned short int *buffer;
 protected:
 void create_buffer(const unsigned long screen_width,const unsigned long screen_height);
 unsigned short int *get_buffer();
 size_t get_length();
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
 fb_fix_screeninfo configuration;
 fb_var_screeninfo setting;
 void read_configuration();
 unsigned long int get_start_offset();
 public:
 SDGF_Screen();
 ~SDGF_Screen();
 void initialize();
 void refresh();
 SDGF_Screen* get_handle();
};

class SDGF_Gamepad
{
 private:
 int device;
 size_t length;
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

class SDGF_File
{
 private:
 FILE *target;
 public:
 SDGF_File();
 ~SDGF_File();
 void open(const char *name);
 void close();
 void set_position(const off_t offset);
 long int get_position();
 long int get_length();
 void read(void *buffer,const size_t length);
 void write(void *buffer,const size_t length);
 bool check_error();
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
 SDGF_Color color;
 SDGF_Screen *surface;
 public:
 SDGF_Primitive();
 ~SDGF_Primitive();
 void initialize(SDGF_Screen *Screen);
 void set_color(const unsigned char red,const unsigned char green,const unsigned char blue);
 void draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2);
 void draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height);
 void draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height);
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
 private:
 unsigned long int width;
 unsigned long int height;
 unsigned long int frames;
 SDGF_Screen *surface;
 void clear_buffer();
 protected:
 SDGF_Color *image;
 void set_width(const unsigned long int image_width);
 void set_height(const unsigned long int image_height);
 SDGF_Color *create_buffer(const unsigned long int image_width,const unsigned long int image_height);
 void draw_image_pixel(const size_t offset,const unsigned long int x,const unsigned long int y);
 size_t get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y);
 public:
 SDGF_Canvas();
 ~SDGF_Canvas();
 SDGF_Color *get_image();
 size_t get_length();
 unsigned long int get_image_width();
 unsigned long int get_image_height();
 void set_frames(const unsigned long int amount);
 unsigned long int get_frames();
 void initialize(SDGF_Screen *Screen);
 void load_image(SDGF_Image &buffer);
 void mirror_image(const SDGF_MIRROR_TYPE kind);
 void resize_image(const unsigned long int new_width,const unsigned long int new_height);
};

class SDGF_Background:public SDGF_Canvas
{
 private:
 unsigned long int start;
 unsigned long int background_width;
 unsigned long int background_height;
 unsigned long int frame;
 SDGF_BACKGROUND_TYPE current_kind;
 public:
 SDGF_Background();
 ~SDGF_Background();
 void set_kind(SDGF_BACKGROUND_TYPE kind);
 void set_target(const unsigned long int target);
 void draw_background();
};

class SDGF_Sprite:public SDGF_Canvas
{
 private:
 unsigned long int current_x;
 unsigned long int current_y;
 unsigned long int sprite_width;
 unsigned long int sprite_height;
 unsigned long int frame;
 unsigned long int start;
 SDGF_SPRITE_TYPE current_kind;
 bool compare_pixels(const SDGF_Color &first,const SDGF_Color &second);
 void draw_sprite_pixel(const size_t offset,const unsigned long int x,const unsigned long int y);
 public:
 SDGF_Sprite();
 ~SDGF_Sprite();
 unsigned long int get_x();
 unsigned long int get_y();
 unsigned long int get_width();
 unsigned long int get_height();
 SDGF_Sprite* get_handle();
 SDGF_Box get_box();
 void set_kind(const SDGF_SPRITE_TYPE kind);
 SDGF_SPRITE_TYPE get_kind();
 void set_target(const unsigned long int target);
 void clone(SDGF_Sprite &target);
 void draw_sprite(const unsigned long int x,const unsigned long int y);
};

class SDGF_Text
{
 private:
 unsigned long int current_x;
 unsigned long int current_y;
 unsigned long int step_x;
 SDGF_Sprite *sprite;
 void draw_character(const char target);
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
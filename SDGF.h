/*
Simple dingux game framework license

Copyright © 2015–2017, Popov Evgeniy Alekseyevich

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
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
#define SDGF_GETRGB565(r,g,b) (b >> 3) +((g >> 2) << 5)+((r >> 3) << 11)

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

struct WAVE_head
{
 char riff_signature[4];
 unsigned long int riff_length:32;
 char wave_signature[4];
 char format[4];
 unsigned long int wave_length:32;
 unsigned short int type:16;
 unsigned short int channels:16;
 unsigned long int rate:32;
 unsigned long int bytes:32;
 unsigned short int align:16;
 unsigned short int bits:16;
 char sample_signature[4];
 unsigned long int sample_length:32;
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

class SDGF_Screen
{
 private:
 int device;
 unsigned long int width;
 unsigned long int height;
 unsigned char color;
 unsigned char *buffer;
 fb_fix_screeninfo configuration;
 fb_var_screeninfo setting;
 public:
 SDGF_Screen();
 ~SDGF_Screen();
 void draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned char red,const unsigned char green,const unsigned char blue);
 void refresh();
 void clear_screen();
 unsigned long int get_width();
 unsigned long int get_height();
 unsigned char get_color();
 SDGF_Screen* get_handle();
};

SDGF_Screen::SDGF_Screen()
{
 device=open("/dev/fb0",O_RDWR);
 if(device==-1)
 {
  puts("Can't get access to frame buffer");
  exit(EXIT_FAILURE);
 }
 if(ioctl(device,FBIOGET_VSCREENINFO,&setting)==-1)
 {
  puts("Can't read framebuffer setting");
  exit(EXIT_FAILURE);
 }
 if(ioctl(device,FBIOGET_FSCREENINFO,&configuration)==-1)
 {
  puts("Can't read framebuffer setting");
  exit(EXIT_FAILURE);
 }
 width=setting.xres;
 height=setting.yres;
 color=setting.bits_per_pixel/8;
 buffer=(unsigned char*)calloc(configuration.smem_len,1);
 if(buffer==NULL)
 {
  puts("Can't allocate memory for render buffer");
  exit(EXIT_FAILURE);
 }

}

SDGF_Screen::~SDGF_Screen()
{
 close(device);
 free(buffer);
}

void SDGF_Screen::draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned char red,const unsigned char green,const unsigned char blue)
{
 unsigned long int offset;
 unsigned short int pixel;
 if ((x<width)&&(y<height))
 {
  offset=(x+setting.xoffset)*color+(y+setting.yoffset)*configuration.line_length;
  pixel=SDGF_GETRGB565(red,green,blue);
  memmove(buffer+offset,&pixel,color);
 }

}

void SDGF_Screen::refresh()
{
 lseek(device,0,SEEK_SET);
 write(device,buffer,configuration.smem_len);
}

void SDGF_Screen::clear_screen()
{
 memset(buffer,0,configuration.smem_len);
}

unsigned long int SDGF_Screen::get_width()
{
 return width;
}

unsigned long int SDGF_Screen::get_height()
{
 return height;
}

unsigned char SDGF_Screen::get_color()
{
 return color;
}

SDGF_Screen* SDGF_Screen::get_handle()
{
 return this;
}

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
 void update();
 unsigned short int get_release();
 unsigned short int get_press();
};

SDGF_Gamepad::SDGF_Gamepad()
{
 device=open("/dev/event0",O_RDONLY|O_NONBLOCK|O_NOCTTY);
 if (device==-1)
 {
  puts("Can't get access to gamepad");
  exit(EXIT_FAILURE);
 }
 length=sizeof(input_event);
 memset(&input,0,length);
 key.button=0;
 key.state=SDGF_GAMEPAD_RELEASE;
}

SDGF_Gamepad::~SDGF_Gamepad()
{
 close(device);
}

void SDGF_Gamepad::update()
{
 key.button=0;
 key.state=SDGF_GAMEPAD_RELEASE;
 while (read(device,&input,length)==length)
 {
  if (input.type==EV_KEY)
  {
   key.button=input.code;
   key.state=input.value;
   if(key.state!=SDGF_GAMEPAD_HOLDING) break;
  }

 }

}

unsigned short int SDGF_Gamepad::get_release()
{
 unsigned short int result;
 result=SDGF_KEY_NONE;
 if(key.state==SDGF_GAMEPAD_RELEASE) result=key.button;
 return result;
}

unsigned short int SDGF_Gamepad::get_press()
{
 unsigned short int result;
 result=SDGF_KEY_NONE;
 if(key.state!=SDGF_GAMEPAD_RELEASE) result=key.button;
 return result;
}

class SDGF_Memory
{
 public:
 unsigned long int get_total_memory();
 unsigned long int get_free_memory();
};

unsigned long int SDGF_Memory::get_total_memory()
{
 unsigned long int memory;
 struct sysinfo information;
 memory=0;
 if (sysinfo(&information)==0) memory=information.totalram*information.mem_unit;
 return memory;
}

unsigned long int SDGF_Memory::get_free_memory()
{
 unsigned long int memory;
 struct sysinfo information;
 memory=0;
 if (sysinfo(&information)==0) memory=information.freeram*information.mem_unit;
 return memory;
}

class SDGF_Sound
{
 private:
 int mixer;
 int sound;
 public:
 SDGF_Sound();
 ~SDGF_Sound();
 void set_volume(int volume);
 int get_volume();
 bool write_data(unsigned char *data, unsigned long int length);
 SDGF_Sound* get_handle();
};

SDGF_Sound::SDGF_Sound()
{
 int setting;
 sound=open("/dev/dsp",O_RDWR|O_EXCL|O_NONBLOCK,S_IRWXU|S_IRWXG|S_IRWXO);
 if (sound==-1)
 {
  puts("Can't get access to sound card");
  exit(EXIT_FAILURE);
 }
 mixer=open("/dev/mixer",O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);
 if (mixer==-1)
 {
  puts("Can't get access to sound mixer");
  exit(EXIT_FAILURE);
 }
 setting=0xffff;
 if(ioctl(mixer,SOUND_MIXER_WRITE_MIC,&setting)==-1)
 {
  puts("Can't configure sound mixer");
  exit(EXIT_FAILURE);
 }
 setting=AFMT_S16_LE;
 if(ioctl(sound,SNDCTL_DSP_SETFMT,&setting)==-1)
 {
  puts("Can't set sound format");
  exit(EXIT_FAILURE);
 }
 setting=2;
 if(ioctl(sound,SNDCTL_DSP_CHANNELS,&setting)==-1)
 {
  puts("Can't set number of audio channels");
  exit(EXIT_FAILURE);
 }
 setting=44100;
 if(ioctl(sound,SNDCTL_DSP_SPEED,&setting)==-1)
 {
  puts("Can't configure sound card");
  exit(EXIT_FAILURE);
 }
 if(ioctl(sound,SNDCTL_DSP_NONBLOCK,NULL)==-1)
 {
  puts("Can't set sample rate");
  exit(EXIT_FAILURE);
 }

}

SDGF_Sound::~SDGF_Sound()
{
 close(sound);
 close(mixer);
}

void SDGF_Sound::set_volume(int volume)
{
 int level;
 if(volume<0) volume=0;
 if(volume>255) volume=255;
 level=(volume << 8)|volume;
 if(ioctl(mixer,SOUND_MIXER_WRITE_VOLUME,&level)==-1)
 {
  puts("Can't set volume level");
  exit(EXIT_FAILURE);
 }

}

int SDGF_Sound::get_volume()
{
 int left,right,volume;
 left=0;
 right=0;
 if(ioctl(mixer,SOUND_MIXER_READ_VOLUME,&volume)>=0)
 {
  left=volume & 0xff;
  right=(volume >> 8) & 0xff;
 }
 return left|right;
}

bool SDGF_Sound::write_data(unsigned char *data, unsigned long int length)
{
 bool result;
 result=true;
 if(write(sound,data,length)<=0)
 {
  if((errno==EAGAIN)||(errno==EBUSY)) result=false;
 }
 return result;
}

SDGF_Sound* SDGF_Sound::get_handle()
{
 return this;
}

class SDGF_Player
{
 private:
 SDGF_Sound *Sound;
 unsigned char *data;
 unsigned long int sample;
 unsigned long int index;
 unsigned long int length;
 public:
 SDGF_Player();
 ~SDGF_Player();
 void unload();
 void load(unsigned char *audio,unsigned long int total,unsigned long int sample_length);
 void initialize(SDGF_Sound *sound);
 bool play();
 void rewind_audio();
};

SDGF_Player::SDGF_Player()
{
 Sound=NULL;
 data=NULL;
 sample=0;
 index=0;
 length=0;
}

SDGF_Player::~SDGF_Player()
{
 Sound=NULL;
 if(data!=NULL) free(data);
}

void SDGF_Player::unload()
{
 sample=0;
 index=0;
 length=0;
 if(data!=NULL) free(data);
}

void SDGF_Player::load(unsigned char *audio,unsigned long int total,unsigned long int sample_length)
{
 this->unload();
 data=audio;
 length=total;
 sample=sample_length;
}

void SDGF_Player::initialize(SDGF_Sound *sound)
{
 Sound=sound;
}

bool SDGF_Player::play()
{
 bool result;
 unsigned long int piece;
 piece=sample;
 result=false;
 if(index<length)
 {
  if(piece>length-index) piece=length-index;
  if(Sound->write_data(&data[index],piece)==true)
  {
   index+=piece;
   result=true;
  }

 }
 return result;
}

void SDGF_Player::rewind_audio()
{
 index=0;
}

class SDGF_Audio
{
 public:
 void load_wave(const char *name,SDGF_Player &player);
};

void SDGF_Audio::load_wave(const char *name,SDGF_Player &player)
{
 FILE *target;
 WAVE_head head;
 unsigned char *data;
 target=fopen(name,"rb");
 if(target==NULL)
 {
  puts("Can't open a sound file");
  exit(EXIT_FAILURE);
 }
 fread(&head,44,1,target);
 if((strncmp(head.riff_signature,"RIFF",4)!=0)&&(strncmp(head.wave_signature,"WAVE",4)!=0))
 {
  puts("Incorrect sound format");
  exit(EXIT_FAILURE);
 }
 if((head.type!=1)&&(head.bits!=16))
 {
  puts("Incorrect sound format");
  exit(EXIT_FAILURE);
 }
 data=(unsigned char*)calloc(head.sample_length,1);
 if(data==NULL)
 {
  puts("Can't allocate memory for sound buffer");
  exit(EXIT_FAILURE);
 }
 fread(data,head.sample_length,1,target);
 fclose(target);
 player.load(data,head.sample_length,head.bytes);
}

class SDGF_System
{
 public:
 SDGF_System();
 ~SDGF_System();
 unsigned long int get_random(const unsigned long int number);
 void pause(const unsigned int long second);
 void quit();
 void run(const char *command);
 char* read_environment(const char *variable);
};

SDGF_System::SDGF_System()
{
 srand(time(NULL));
}

SDGF_System::~SDGF_System()
{

}

unsigned long int SDGF_System::get_random(const unsigned long int number)
{
 return rand()%number;
}

void SDGF_System::pause(const unsigned int long second)
{
 time_t start,stop;
 start=time(NULL);
 do
 {
  stop=time(NULL);
 } while(difftime(stop,start)<second);

}

void SDGF_System::quit()
{
 exit(EXIT_SUCCESS);
}

void SDGF_System::run(const char *command)
{
 system(command);
}

char* SDGF_System::read_environment(const char *variable)
{
 return getenv(variable);
}

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

SDGF_Timer::SDGF_Timer()
{
 interval=0;
 start=time(NULL);
}

SDGF_Timer::~SDGF_Timer()
{

}

void SDGF_Timer::set_timer(const unsigned long int seconds)
{
 interval=seconds;
 start=time(NULL);
}

bool SDGF_Timer::check_timer()
{
 bool result;
 time_t stop;
 result=false;
 stop=time(NULL);
 if(difftime(stop,start)>=interval)
 {
  result=true;
  start=time(NULL);
 }
 return result;
}

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

SDGF_Primitive::SDGF_Primitive()
{
 surface=NULL;
}

SDGF_Primitive::~SDGF_Primitive()
{
 surface=NULL;
}

void SDGF_Primitive::initialize(SDGF_Screen *Screen)
{
 surface=Screen;
}

void SDGF_Primitive::draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2,const unsigned char red,const unsigned char green,const unsigned char blue)
{
 unsigned long int delta_x,delta_y,index,steps;
 float x,y,shift_x,shift_y;
 if (x1>x2)
 {
  delta_x=x1-x2;
 }
 else
 {
  delta_x=x2-x1;
 }
 if (y1>y2)
 {
  delta_y=y1-y2;
 }
 else
 {
  delta_y=y2-y1;
 }
 steps=delta_x;
 if (steps<delta_y) steps=delta_y;
 x=x1;
 y=y1;
 shift_x=(float)delta_x/(float)steps;
 shift_y=(float)delta_y/(float)steps;
 for (index=steps;index>0;index--)
 {
  x+=shift_x;
  y+=shift_y;
  surface->draw_pixel(x,y,red,green,blue);
 }

}

void SDGF_Primitive::draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height,const unsigned char red,const unsigned char green,const unsigned char blue)
{
 unsigned long int stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 this->draw_line(x,y,stop_x,y,red,green,blue);
 this->draw_line(x,stop_y,stop_x,stop_y,red,green,blue);
 this->draw_line(x,y,x,stop_y,red,green,blue);
 this->draw_line(stop_x,y,stop_x,stop_y,red,green,blue);
}

void SDGF_Primitive::draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height,const unsigned char red,const unsigned char green,const unsigned char blue)
{
 unsigned long int step_x,step_y,stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 for(step_x=x;step_x<stop_x;step_x++)
 {
  for(step_y=y;step_y<stop_y;step_y++)
  {
   surface->draw_pixel(step_x,step_y,red,green,blue);
  }

 }

}

class SDGF_Image
{
 private:
 unsigned long int width;
 unsigned long int height;
 unsigned char *data;
 public:
 SDGF_Image();
 ~SDGF_Image();
 void load_tga(const char *name);
 void load_pcx(const char *name);
 unsigned long int get_width();
 unsigned long int get_height();
 unsigned long int get_data_length();
 unsigned char *get_data();
 void destroy_image();
};

SDGF_Image::SDGF_Image()
{
 width=0;
 height=0;
 data=NULL;
}

SDGF_Image::~SDGF_Image()
{
 if(data!=NULL) free(data);
}

void SDGF_Image::load_tga(const char *name)
{
 FILE *target;
 unsigned long int index,position,amount,compressed_length,uncompressed_length;
 unsigned char *compressed;
 unsigned char *uncompressed;
 TGA_head head;
 TGA_map color_map;
 TGA_image image;
 target=fopen(name,"rb");
 if(target==NULL)
 {
  puts("Can't open a image file");
  exit(EXIT_FAILURE);
 }
 if(data!=NULL)
 {
  free(data);
  data=NULL;
 }
 fseek(target,0,SEEK_END);
 compressed_length=ftell(target)-18;
 rewind(target);
 fread(&head,3,1,target);
 fread(&color_map,5,1,target);
 fread(&image,10,1,target);
 if((head.color_map!=0)||(image.color!=24))
 {
  puts("Invalid image format");
  exit(EXIT_FAILURE);
 }
 if(head.type!=2)
 {
  if(head.type!=10)
  {
   puts("Invalid image format");
   exit(EXIT_FAILURE);
  }

 }
 index=0;
 position=0;
 uncompressed_length=3*(unsigned long int)image.width*(unsigned long int)image.height;
 uncompressed=(unsigned char*)calloc(uncompressed_length,1);
 if(uncompressed==NULL)
 {
  puts("Can't allocate memory for image buffer");
  exit(EXIT_FAILURE);
 }
 if(head.type==2)
 {
  fread(uncompressed,uncompressed_length,1,target);
 }
 if(head.type==10)
 {
  compressed=(unsigned char*)calloc(compressed_length,1);
  if(compressed==NULL)
  {
   puts("Can't allocate memory for image buffer");
   exit(EXIT_FAILURE);
  }
  fread(compressed,compressed_length,1,target);
  while(index<uncompressed_length)
  {
   if(compressed[position]<128)
   {
    amount=compressed[position]+1;
    amount*=3;
    memmove(uncompressed+index,compressed+(position+1),amount);
    index+=amount;
    position+=1+amount;
   }
   else
   {
    for(amount=compressed[position]-127;amount>0;amount--)
    {
     memmove(uncompressed+index,compressed+(position+1),3);
     index+=3;
    }
    position+=4;
   }

  }
  free(compressed);
 }
 fclose(target);
 width=image.width;
 height=image.height;
 data=uncompressed;
}

void SDGF_Image::load_pcx(const char *name)
{
 FILE *target;
 unsigned long int x,y,index,position,line,row,length,uncompressed_length;
 unsigned char repeat;
 unsigned char *original;
 unsigned char *uncompressed;
 PCX_head head;
 target=fopen(name,"rb");
 if(target==NULL)
 {
  puts("Can't open a image file");
  exit(EXIT_FAILURE);
 }
 if(data!=NULL)
 {
  free(data);
  data=NULL;
 }
 fseek(target,0,SEEK_END);
 length=ftell(target)-128;
 rewind(target);
 fread(&head,128,1,target);
 if((head.color*head.planes!=24)&&(head.compress!=1))
 {
  puts("Incorrect image format");
  exit(EXIT_FAILURE);
 }
 width=head.max_x-head.min_x+1;
 height=head.max_y-head.min_y+1;
 row=3*width;
 line=head.planes*head.plane_length;
 uncompressed_length=row*height;
 index=0;
 position=0;
 original=(unsigned char*)calloc(length,1);
 if(original==NULL)
 {
  puts("Can't allocate memory for image buffer");
  exit(EXIT_FAILURE);
 }
 uncompressed=(unsigned char*)calloc(uncompressed_length,1);
 if(uncompressed==NULL)
 {
  puts("Can't allocate memory for image buffer");
  exit(EXIT_FAILURE);
 }
 fread(original,length,1,target);
 fclose(target);
 while (index<length)
 {
  if (original[index]<192)
  {
   uncompressed[position]=original[index];
   position++;
   index++;
  }
  else
  {
   for (repeat=original[index]-192;repeat>0;repeat--)
   {
    uncompressed[position]=original[index+1];
    position++;
   }
   index+=2;
  }

 }
 free(original);
 original=(unsigned char*)calloc(uncompressed_length,1);
 if(original==NULL)
 {
  puts("Can't allocate memory for image buffer");
  exit(EXIT_FAILURE);
 }
 for(x=0;x<width;x++)
 {
  for(y=0;y<height;y++)
  {
   index=x*3+y*row;
   position=x+y*line;
   original[index]=uncompressed[position+2*head.plane_length];
   original[index+1]=uncompressed[position+head.plane_length];
   original[index+2]=uncompressed[position];
  }

 }
 free(uncompressed);
 data=original;
}

unsigned long int SDGF_Image::get_width()
{
 return width;
}

unsigned long int SDGF_Image::get_height()
{
 return height;
}

unsigned long int SDGF_Image::get_data_length()
{
 return width*height*3;
}

unsigned char *SDGF_Image::get_data()
{
 return data;
}

void SDGF_Image::destroy_image()
{
 if(data!=NULL)
 {
  width=0;
  height=0;
  free(data);
  data=NULL;
 }

}

class SDGF_Canvas
{
 protected:
 unsigned long int width;
 unsigned long int height;
 unsigned long int frames;
 SDGF_Screen *surface;
 SDGF_Color *image;
 public:
 SDGF_Canvas();
 ~SDGF_Canvas();
 unsigned long int get_width();
 unsigned long int get_height();
 void set_frames(const unsigned long int amount);
 unsigned long int get_frames();
 void initialize(SDGF_Screen *Screen);
 void load_image(SDGF_Image &buffer);
 void mirror_image(const unsigned char kind);
 void resize_image(const unsigned long int new_width,const unsigned long int new_height);
};

SDGF_Canvas::SDGF_Canvas()
{
 image=NULL;
 surface=NULL;
 width=0;
 height=0;
 frames=1;
}

SDGF_Canvas::~SDGF_Canvas()
{
 surface=NULL;
 if(image!=NULL) free(image);
}

unsigned long int SDGF_Canvas::get_width()
{
 return width;
}

unsigned long int SDGF_Canvas::get_height()
{
 return height;
}

void SDGF_Canvas::set_frames(const unsigned long int amount)
{
 if(amount>1) frames=amount;
}

unsigned long int SDGF_Canvas::get_frames()
{
 return frames;
}

void SDGF_Canvas::initialize(SDGF_Screen *Screen)
{
 surface=Screen;
}

void SDGF_Canvas::load_image(SDGF_Image &buffer)
{
 unsigned long int length;
 width=buffer.get_width();
 height=buffer.get_height();
 length=buffer.get_data_length();
 if(image!=NULL) free(image);
 image=(SDGF_Color*)calloc(length,1);
 if (image==NULL)
 {
  puts("Can't allocate memory for image buffer");
  exit(EXIT_FAILURE);
 }
 memmove(image,buffer.get_data(),length);
 buffer.destroy_image();
}

void SDGF_Canvas::mirror_image(const unsigned char kind)
{
 unsigned long int x,y,index,index2;
 SDGF_Color *mirrored_image;
 mirrored_image=(SDGF_Color*)calloc(width*height,3);
 if (mirrored_image==NULL)
 {
  puts("Can't allocate memory for image buffer");
  exit(EXIT_FAILURE);
 }
 if (kind==0)
 {
  for (x=0;x<width;x++)
  {
   for (y=0;y<height;y++)
   {
    index=x+(y*width);
    index2=(width-x-1)+(y*width);
    mirrored_image[index]=image[index2];
   }

  }

 }
 else
 {
   for (x=0;x<width;x++)
  {
   for (y=0;y<height;y++)
   {
    index=x+(y*width);
    index2=x+(height-y-1)*width;
    mirrored_image[index]=image[index2];
   }

  }

 }
 free(image);
 image=mirrored_image;
}

void SDGF_Canvas::resize_image(const unsigned long int new_width,const unsigned long int new_height)
{
 float x_ratio,y_ratio;
 unsigned long int x,y,index,index2;
 SDGF_Color *scaled_image;
 scaled_image=(SDGF_Color*)calloc(new_width*new_height,3);
 if (scaled_image==NULL)
 {
  puts("Can't allocate memory for image buffer");
  exit(EXIT_FAILURE);
 }
 x_ratio=(float)width/(float)new_width;
 y_ratio=(float)height/(float)new_height;
 for (x=0;x<new_width;x++)
 {
  for (y=0;y<new_height;y++)
  {
   index=x+(y*new_width);
   index2=(unsigned long int)(x_ratio*(float)x)+width*(unsigned long int)(y_ratio*(float)y);
   scaled_image[index]=image[index2];
  }

 }
 free(image);
 image=scaled_image;
 width=new_width;
 height=new_height;
}

class SDGF_Background:public SDGF_Canvas
{
 public:
 void draw_background();
 void draw_horizontal_background(const unsigned long int frame);
 void draw_vertical_background(const unsigned long int frame);
};

void SDGF_Background::draw_background()
{
 unsigned long int x,y,offset;
 for (x=0;x<width;x++)
 {
  for (y=0;y<height;y++)
  {
   offset=x+(width*y);
   surface->draw_pixel(x,y,image[offset].red,image[offset].green,image[offset].blue);
  }

 }

}

void SDGF_Background::draw_horizontal_background(const unsigned long int frame)
{
 unsigned long int x,y,offset,start,frame_width;
 frame_width=width/frames;
 start=(frame-1)*frame_width;
 for (x=0;x<frame_width;x++)
 {
  for (y=0;y<height;y++)
  {
   offset=start+x+(width*y);
   surface->draw_pixel(x,y,image[offset].red,image[offset].green,image[offset].blue);
  }

 }

}

void SDGF_Background::draw_vertical_background(const unsigned long int frame)
{
 unsigned long int x,y,offset,start,frame_height;
 frame_height=height/frames;
 start=(frame-1)*frame_height;
 for (x=0;x<width;x++)
 {
  for (y=0;y<frame_height;y++)
  {
   offset=start+x+(width*y);
   surface->draw_pixel(x,y,image[offset].red,image[offset].green,image[offset].blue);
  }

 }

}

class SDGF_Sprite:public SDGF_Canvas
{
 private:
 unsigned long int current_x;
 unsigned long int current_y;
 public:
 SDGF_Sprite();
 ~SDGF_Sprite();
 void draw_sprite_frame(const unsigned long int x,const unsigned long int y,const unsigned long int frame);
 void draw_sprite(const unsigned long int x,const unsigned long int y);
 unsigned long int get_x();
 unsigned long int get_y();
 unsigned long int get_sprite_width();
 unsigned long int get_sprite_height();
 SDGF_Sprite* get_handle();
};

SDGF_Sprite::SDGF_Sprite()
{
 current_x=0;
 current_y=0;
}

SDGF_Sprite::~SDGF_Sprite()
{

}

void SDGF_Sprite::draw_sprite_frame(const unsigned long int x,const unsigned long int y,const unsigned long int frame)
{
 unsigned long int sprite_x,sprite_y,offset,start,frame_width;
 current_x=x;
 current_y=y;
 frame_width=width/frames;
 start=(frame-1)*frame_width;
 for(sprite_x=0;sprite_x<frame_width;sprite_x++)
 {
  for(sprite_y=0;sprite_y<height;sprite_y++)
  {
   offset=start+sprite_x+(sprite_y*width);
   if(memcmp(&image[0],&image[offset],3)!=0) surface->draw_pixel(x+sprite_x,y+sprite_y,image[offset].red,image[offset].green,image[offset].blue);
  }

 }

}

void SDGF_Sprite::draw_sprite(const unsigned long int x,const unsigned long int y)
{
 current_x=x;
 current_y=y;
 this->draw_sprite_frame(x,y,1);
}

unsigned long int SDGF_Sprite::get_x()
{
 return current_x;
}

unsigned long int SDGF_Sprite::get_y()
{
 return current_y;
}

unsigned long int SDGF_Sprite::get_sprite_width()
{
 return width/frames;
}

unsigned long int SDGF_Sprite::get_sprite_height()
{
 return height;
}

SDGF_Sprite* SDGF_Sprite::get_handle()
{
 return this;
}

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

SDGF_Text::SDGF_Text()
{
 current_x=0;
 current_y=0;
 sprite=NULL;
}

SDGF_Text::~SDGF_Text()
{
 sprite=NULL;
}

void SDGF_Text::set_position(const unsigned long int x,const unsigned long int y)
{
 current_x=x;
 current_y=y;
}

void SDGF_Text::load_font(SDGF_Sprite *font)
{
 sprite=font;
 sprite->set_frames(128);
}

void SDGF_Text::draw_text(const char *text)
{
 unsigned long int index,length,step_x,step_y;
 length=strlen(text);
 step_x=current_x;
 step_y=current_y;
 for (index=0;index<length;index++)
 {
  if (text[index]>31)
  {
   sprite->draw_sprite_frame(step_x,step_y,text[index]+1);
   step_x+=sprite->get_sprite_width();
  }

 }

}

class SDGF_Collision
{
 public:
 bool check_horizontal_collision(SDGF_Sprite &first,SDGF_Sprite &second);
 bool check_vertical_collision(SDGF_Sprite &first,SDGF_Sprite &second);
 bool check_collision(SDGF_Sprite &first,SDGF_Sprite &second);
};

bool SDGF_Collision::check_horizontal_collision(SDGF_Sprite &first,SDGF_Sprite &second)
{
 bool result;
 result=false;
 if((first.get_x()+first.get_sprite_width())>=second.get_x())
 {
  if(first.get_x()<=(second.get_x()+second.get_sprite_width())) result=true;
 }
 return result;
}

bool SDGF_Collision::check_vertical_collision(SDGF_Sprite &first,SDGF_Sprite &second)
{
 bool result;
 result=false;
 if((first.get_y()+first.get_sprite_height())>=second.get_y())
 {
  if(first.get_y()<=(second.get_y()+second.get_sprite_height())) result=true;
 }
 return result;
}

bool SDGF_Collision::check_collision(SDGF_Sprite &first,SDGF_Sprite &second)
{
 bool result;
 result=false;
 if((this->check_horizontal_collision(first,second)==true)||(this->check_vertical_collision(first,second)==true)) result=true;
 return result;
}
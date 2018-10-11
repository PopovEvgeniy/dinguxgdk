// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "SDGF.h"

void SDGF_Show_Error(const char *message)
{
 puts(message);
 exit(EXIT_FAILURE);
}

SDGF_Frame::SDGF_Frame()
{
 width=0;
 height=0;
 length=0;
 buffer=NULL;
}

SDGF_Frame::~SDGF_Frame()
{
 if(buffer!=NULL) free(buffer);
}

void SDGF_Frame::create_buffer(const unsigned long screen_width,const unsigned long screen_height)
{
 width=screen_width;
 height=screen_height;
 length=(size_t)width*(size_t)height;
 buffer=(unsigned short int*)calloc(length,sizeof(unsigned short int));
 if(buffer==NULL)
 {
  SDGF_Show_Error("Can't allocate memory for render buffer");
 }
 length*=sizeof(unsigned short int);
}

unsigned short int *SDGF_Frame::get_buffer()
{
 return buffer;
}

size_t SDGF_Frame::get_length()
{
 return length;
}

unsigned short int SDGF_Frame::get_bgr565(const unsigned char red,const unsigned char green,const unsigned char blue)
{
 return (blue >> 3) +((green >> 2) << 5)+((red >> 3) << 11); // This code bases on code from SVGALib
}

void SDGF_Frame::draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned char red,const unsigned char green,const unsigned char blue)
{
 if ((x<width)&&(y<height))
 {
  buffer[(size_t)x+(size_t)y*(size_t)width]=this->get_bgr565(red,green,blue);
 }

}

void SDGF_Frame::clear_screen()
{
 memset(buffer,0,length);
}

unsigned long int SDGF_Frame::get_width()
{
 return width;
}

unsigned long int SDGF_Frame::get_height()
{
 return height;
}

SDGF_Screen::SDGF_Screen()
{
 device=open("/dev/fb0",O_RDWR);
 if(device==-1)
 {
  SDGF_Show_Error("Can't get access to frame buffer");
 }
 memset(&setting,0,sizeof(FBIOGET_VSCREENINFO));
 memset(&configuration,0,sizeof(FBIOGET_FSCREENINFO));
}

SDGF_Screen::~SDGF_Screen()
{
 if(device!=-1) close(device);
}

void SDGF_Screen::read_configuration()
{
 if(ioctl(device,FBIOGET_VSCREENINFO,&setting)==-1)
 {
  SDGF_Show_Error("Can't read framebuffer setting");
 }
 if(ioctl(device,FBIOGET_FSCREENINFO,&configuration)==-1)
 {
  SDGF_Show_Error("Can't read framebuffer setting");
 }

}

unsigned long int SDGF_Screen::get_start_offset()
{
 return setting.xoffset*(setting.bits_per_pixel/8)+setting.yoffset*configuration.line_length;
}

void SDGF_Screen::initialize()
{
 this->read_configuration();
 this->create_buffer(setting.xres,setting.yres);
 start=this->get_start_offset();
}

void SDGF_Screen::refresh()
{
 lseek(device,start,SEEK_SET);
 write(device,this->get_buffer(),this->get_length());
}

SDGF_Screen* SDGF_Screen::get_handle()
{
 return this;
}

SDGF_Gamepad::SDGF_Gamepad()
{
 length=sizeof(input_event);
 memset(&input,0,length);
}

SDGF_Gamepad::~SDGF_Gamepad()
{
 if(device!=-1) close(device);
}

void SDGF_Gamepad::initialize()
{
 device=open("/dev/event0",O_RDONLY|O_NONBLOCK|O_NOCTTY);
 if (device==-1)
 {
  SDGF_Show_Error("Can't get access to gamepad");
 }
 key.button=0;
 key.state=SDGF_GAMEPAD_RELEASE;
}

void SDGF_Gamepad::update()
{
 key.button=0;
 key.state=SDGF_GAMEPAD_RELEASE;
 while (read(device,&input,length)>0)
 {
  if (input.type==EV_KEY)
  {
   key.button=input.code;
   key.state=input.value;
   if(key.state!=SDGF_GAMEPAD_HOLDING) break;
  }

 }

}

unsigned short int SDGF_Gamepad::get_hold()
{
 unsigned short int result;
 result=SDGF_KEY_NONE;
 if(key.state!=SDGF_GAMEPAD_RELEASE) result=key.button;
 return result;
}

unsigned short int SDGF_Gamepad::get_press()
{
 unsigned short int result;
 result=SDGF_KEY_NONE;
 if(key.state==SDGF_GAMEPAD_PRESS) result=key.button;
 return result;
}

unsigned short int SDGF_Gamepad::get_release()
{
 unsigned short int result;
 result=SDGF_KEY_NONE;
 if(key.state==SDGF_GAMEPAD_RELEASE) result=key.button;
 return result;
}

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

void SDGF_System::enable_logging(const char *name)
{
 if(freopen(name,"wt",stdout)==NULL)
 {
  SDGF_Show_Error("Can't create log file");
 }

}

SDGF_File::SDGF_File()
{
 target=NULL;
}

SDGF_File::~SDGF_File()
{
 if(target!=NULL) fclose(target);
}

void SDGF_File::open(const char *name)
{
 target=fopen(name,"w+b");
 if(target==NULL)
 {
  SDGF_Show_Error("Can't open the binary file");
 }

}

void SDGF_File::close()
{
 if(target!=NULL) fclose(target);
}

void SDGF_File::set_position(const off_t offset)
{
 fseek(target,offset,SEEK_SET);
}

long int SDGF_File::get_position()
{
 return ftell(target);
}

long int SDGF_File::get_length()
{
 long int result;
 fseek(target,0,SEEK_END);
 result=ftell(target);
 rewind(target);
 return result;
}

void SDGF_File::read(void *buffer,const size_t length)
{
 fread(buffer,length,1,target);
}

void SDGF_File::write(void *buffer,const size_t length)
{
 fwrite(buffer,length,1,target);
}

bool SDGF_File::check_error()
{
 bool result;
 result=false;
 if(ferror(target)!=0) result=true;
 return result;
}

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

SDGF_Primitive::SDGF_Primitive()
{
 surface=NULL;
 color.red=0;
 color.green=0;
 color.blue=0;
}

SDGF_Primitive::~SDGF_Primitive()
{
 color.red=0;
 color.green=0;
 color.blue=0;
 surface=NULL;
}

void SDGF_Primitive::initialize(SDGF_Screen *Screen)
{
 surface=Screen;
}

void SDGF_Primitive::set_color(const unsigned char red,const unsigned char green,const unsigned char blue)
{
 color.red=red;
 color.green=green;
 color.blue=blue;
}

void SDGF_Primitive::draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2)
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
 for (index=steps;index>0;--index)
 {
  x+=shift_x;
  y+=shift_y;
  surface->draw_pixel(x,y,color.red,color.green,color.blue);
 }

}

void SDGF_Primitive::draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 this->draw_line(x,y,stop_x,y);
 this->draw_line(x,stop_y,stop_x,stop_y);
 this->draw_line(x,y,x,stop_y);
 this->draw_line(stop_x,y,stop_x,stop_y);
}

void SDGF_Primitive::draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int step_x,step_y,stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 for(step_x=x;step_x<stop_x;++step_x)
 {
  for(step_y=y;step_y<stop_y;++step_y)
  {
   surface->draw_pixel(step_x,step_y,color.red,color.green,color.blue);
  }

 }

}

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

unsigned char *SDGF_Image::create_buffer(const size_t length)
{
 unsigned char *result;
 result=(unsigned char*)calloc(length,sizeof(unsigned char));
 if(result==NULL)
 {
  SDGF_Show_Error("Can't allocate memory for image buffer");
 }
 return result;
}

void SDGF_Image::clear_buffer()
{
 if(data!=NULL)
 {
  free(data);
  data=NULL;
 }

}

FILE *SDGF_Image::open_image(const char *name)
{
 FILE *target;
 target=fopen(name,"rb");
 if(target==NULL)
 {
  SDGF_Show_Error("Can't open a image file");
 }
 return target;
}

unsigned long int SDGF_Image::get_file_size(FILE *target)
{
 unsigned long int length;
 fseek(target,0,SEEK_END);
 length=ftell(target);
 rewind(target);
 return length;
}

void SDGF_Image::load_tga(const char *name)
{
 FILE *target;
 size_t index,position,amount,compressed_length,uncompressed_length;
 unsigned char *compressed;
 unsigned char *uncompressed;
 TGA_head head;
 TGA_map color_map;
 TGA_image image;
 this->clear_buffer();
 target=this->open_image(name);
 compressed_length=(size_t)this->get_file_size(target)-18;
 fread(&head,3,1,target);
 fread(&color_map,5,1,target);
 fread(&image,10,1,target);
 if((head.color_map!=0)||(image.color!=24))
 {
  SDGF_Show_Error("Invalid image format");
 }
 if(head.type!=2)
 {
  if(head.type!=10)
  {
   SDGF_Show_Error("Invalid image format");
  }

 }
 index=0;
 position=0;
 width=image.width;
 height=image.height;
 uncompressed_length=this->get_data_length();
 uncompressed=this->create_buffer(uncompressed_length);
 if(head.type==2)
 {
  fread(uncompressed,uncompressed_length,1,target);
 }
 if(head.type==10)
 {
  compressed=this->create_buffer(compressed_length);
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
    for(amount=compressed[position]-127;amount>0;--amount)
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
 data=uncompressed;
}

void SDGF_Image::load_pcx(const char *name)
{
 FILE *target;
 unsigned long int x,y;
 size_t index,position,line,row,length,uncompressed_length;
 unsigned char repeat;
 unsigned char *original;
 unsigned char *uncompressed;
 PCX_head head;
 this->clear_buffer();
 target=this->open_image(name);
 length=(size_t)this->get_file_size(target)-128;
 fread(&head,128,1,target);
 if((head.color*head.planes!=24)&&(head.compress!=1))
 {
  SDGF_Show_Error("Incorrect image format");
 }
 width=head.max_x-head.min_x+1;
 height=head.max_y-head.min_y+1;
 row=3*(size_t)width;
 line=(size_t)head.planes*(size_t)head.plane_length;
 uncompressed_length=row*height;
 index=0;
 position=0;
 original=this->create_buffer(length);
 uncompressed=this->create_buffer(uncompressed_length);
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
   for (repeat=original[index]-192;repeat>0;--repeat)
   {
    uncompressed[position]=original[index+1];
    position++;
   }
   index+=2;
  }

 }
 free(original);
 original=this->create_buffer(uncompressed_length);
 for(x=0;x<width;++x)
 {
  for(y=0;y<height;++y)
  {
   index=(size_t)x*3+(size_t)y*row;
   position=(size_t)x+(size_t)y*line;
   original[index]=uncompressed[position+2*(size_t)head.plane_length];
   original[index+1]=uncompressed[position+(size_t)head.plane_length];
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

size_t SDGF_Image::get_data_length()
{
 return (size_t)width*(size_t)height*3;
}

unsigned char *SDGF_Image::get_data()
{
 return data;
}

void SDGF_Image::destroy_image()
{
 width=0;
 height=0;
 this->clear_buffer();
}

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

void SDGF_Canvas::clear_buffer()
{
 if(image!=NULL) free(image);
}

void SDGF_Canvas::set_width(const unsigned long int image_width)
{
 width=image_width;
}

void SDGF_Canvas::set_height(const unsigned long int image_height)
{
 height=image_height;
}

SDGF_Color *SDGF_Canvas::create_buffer(const unsigned long int image_width,const unsigned long int image_height)
{
 SDGF_Color *result;
 size_t length;
 length=(size_t)image_width*(size_t)image_height;
 result=(SDGF_Color*)calloc(length,3);
 if(result==NULL)
 {
  SDGF_Show_Error("Can't allocate memory for image buffer");
 }
 return result;
}

void SDGF_Canvas::draw_image_pixel(const size_t offset,const unsigned long int x,const unsigned long int y)
{
 surface->draw_pixel(x,y,image[offset].red,image[offset].green,image[offset].blue);
}

size_t SDGF_Canvas::get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y)
{
 return (size_t)start+(size_t)x+(size_t)y*(size_t)width;
}

SDGF_Color *SDGF_Canvas::get_image()
{
 return image;
}

size_t SDGF_Canvas::get_length()
{
 return (size_t)width*(size_t)height;
}

unsigned long int SDGF_Canvas::get_image_width()
{
 return width;
}

unsigned long int SDGF_Canvas::get_image_height()
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
 width=buffer.get_width();
 height=buffer.get_height();
 this->clear_buffer();
 image=this->create_buffer(width,height);
 memmove(image,buffer.get_data(),buffer.get_data_length());
 buffer.destroy_image();
}

void SDGF_Canvas::mirror_image(const SDGF_MIRROR_TYPE kind)
{
 unsigned long int x,y;
 size_t index,index2;
 SDGF_Color *mirrored_image;
 mirrored_image=this->create_buffer(width,height);
 if (kind==SDGF_MIRROR_HORIZONTAL)
 {
  for (x=0;x<width;++x)
  {
   for (y=0;y<height;++y)
   {
    index=this->get_offset(0,x,y);
    index2=this->get_offset(0,(width-x-1),y);
    mirrored_image[index]=image[index2];
   }

  }

 }
 if(kind==SDGF_MIRROR_VERTICAL)
 {
   for (x=0;x<width;++x)
  {
   for (y=0;y<height;++y)
   {
    index=this->get_offset(0,x,y);
    index2=this->get_offset(0,x,(height-y-1));
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
 unsigned long int x,y;
 size_t index,index2;
 SDGF_Color *scaled_image;
 scaled_image=this->create_buffer(new_width,new_height);
 x_ratio=(float)width/(float)new_width;
 y_ratio=(float)height/(float)new_height;
 for (x=0;x<new_width;++x)
 {
  for (y=0;y<new_height;++y)
  {
   index=(size_t)x+(size_t)y*(size_t)new_width;
   index2=(size_t)(x_ratio*(float)x)+(size_t)width*(size_t)(y_ratio*(float)y);
   scaled_image[index]=image[index2];
  }

 }
 free(image);
 image=scaled_image;
 width=new_width;
 height=new_height;
}

SDGF_Background::SDGF_Background()
{
 start=0;
 background_width=0;
 background_height=0;
 frame=1;
 current_kind=SDGF_NORMAL_BACKGROUND;
}

SDGF_Background::~SDGF_Background()
{

}

void SDGF_Background::set_kind(SDGF_BACKGROUND_TYPE kind)
{
 switch(kind)
 {
  case SDGF_NORMAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height();
  start=0;
  break;
  case SDGF_HORIZONTAL_BACKGROUND:
  background_width=this->get_image_width()/this->get_frames();
  background_height=this->get_image_height();
  start=(frame-1)*background_width;
  break;
  case SDGF_VERTICAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height()/this->get_frames();
  start=(frame-1)*background_width*background_height;
  break;
 }
 current_kind=kind;
}

void SDGF_Background::set_target(const unsigned long int target)
{
 if((target>0)&&(target<=this->get_frames()))
 {
  frame=target;
  this->set_kind(current_kind);
 }

}

void SDGF_Background::draw_background()
{
 unsigned long int x,y;
 size_t offset;
 for(x=0;x<background_width;++x)
 {
  for(y=0;y<background_height;++y)
  {
   offset=this->get_offset(start,x,y);
   this->draw_image_pixel(offset,x,y);
  }

 }

}

SDGF_Sprite::SDGF_Sprite()
{
 current_x=0;
 current_y=0;
 sprite_width=0;
 sprite_height=0;
 frame=0;
 start=0;
 current_kind=SDGF_SINGE_SPRITE;
}

SDGF_Sprite::~SDGF_Sprite()
{

}

bool SDGF_Sprite::compare_pixels(const SDGF_Color &first,const SDGF_Color &second)
{
 bool result;
 result=false;
 if ((first.red!=second.red)||(first.green!=second.green))
 {
  result=true;
 }
 else
 {
  if(first.blue!=second.blue) result=true;
 }
 return result;
}

void SDGF_Sprite::draw_sprite_pixel(const size_t offset,const unsigned long int x,const unsigned long int y)
{
 if(this->compare_pixels(image[0],image[offset])==true) this->draw_image_pixel(offset,x,y);
}

unsigned long int SDGF_Sprite::get_x()
{
 return current_x;
}

unsigned long int SDGF_Sprite::get_y()
{
 return current_y;
}

unsigned long int SDGF_Sprite::get_width()
{
 return sprite_width;
}

unsigned long int SDGF_Sprite::get_height()
{
 return sprite_height;
}

SDGF_Sprite* SDGF_Sprite::get_handle()
{
 return this;
}

SDGF_Box SDGF_Sprite::get_box()
{
 SDGF_Box target;
 target.x=current_x;
 target.y=current_y;
 target.width=sprite_width;
 target.height=sprite_height;
 return target;
}

void SDGF_Sprite::set_kind(const SDGF_SPRITE_TYPE kind)
{
 switch(kind)
 {
  case SDGF_SINGE_SPRITE:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height();
  start=0;
  break;
  case SDGF_HORIZONTAL_STRIP:
  sprite_width=this->get_image_width()/this->get_frames();
  sprite_height=this->get_image_height();
  start=(frame-1)*sprite_width;
  break;
  case SDGF_VERTICAL_STRIP:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height()/this->get_frames();
  start=(frame-1)*sprite_width*sprite_height;
  break;
 }
 current_kind=kind;
}

SDGF_SPRITE_TYPE SDGF_Sprite::get_kind()
{
 return current_kind;
}

void SDGF_Sprite::set_target(const unsigned long int target)
{
 if((target>0)&&(target<=this->get_frames()))
 {
  frame=target;
  this->set_kind(current_kind);
 }

}

void SDGF_Sprite::set_position(const unsigned long int x,const unsigned long int y)
{
 current_x=x;
 current_y=y;
}

void SDGF_Sprite::clone(SDGF_Sprite &target)
{
 this->set_width(target.get_image_width());
 this->set_height(target.get_image_height());
 this->set_frames(target.get_frames());
 this->set_kind(target.get_kind());
 image=this->create_buffer(target.get_image_width(),target.get_image_width());
 memmove(image,target.get_image(),target.get_length());
}

void SDGF_Sprite::draw_sprite()
{
 size_t offset;
 unsigned long int sprite_x,sprite_y;
 for(sprite_x=0;sprite_x<sprite_width;++sprite_x)
 {
  for(sprite_y=0;sprite_y<sprite_height;++sprite_y)
  {
   offset=this->get_offset(start,sprite_x,sprite_y);
   this->draw_sprite_pixel(offset,current_x+sprite_x,current_y+sprite_y);
  }

 }

}

SDGF_Text::SDGF_Text()
{
 current_x=0;
 current_y=0;
 step_x=0;
 sprite=NULL;
}

SDGF_Text::~SDGF_Text()
{
 sprite=NULL;
}

void SDGF_Text::draw_character(const char target)
{
 if(target>31)
 {
  sprite->set_target((unsigned long int)target+1);
  sprite->set_position(step_x,current_y);
  sprite->draw_sprite();
  step_x+=sprite->get_width();
 }

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
 sprite->set_kind(SDGF_HORIZONTAL_STRIP);
}

void SDGF_Text::draw_text(const char *text)
{
 size_t index,length;
 length=strlen(text);
 step_x=current_x;
 for (index=0;index<length;++index)
 {
  this->draw_character(text[index]);
 }

}

bool SDGF_Collision::check_horizontal_collision(const SDGF_Box &first,const SDGF_Box &second)
{
 bool result;
 result=false;
 if((first.x+first.width)>=second.x)
 {
  if(first.x<=(second.x+second.width)) result=true;
 }
 return result;
}

bool SDGF_Collision::check_vertical_collision(const SDGF_Box &first,const SDGF_Box &second)
{
 bool result;
 result=false;
 if((first.y+first.height)>=second.y)
 {
  if(first.y<=(second.y+second.height)) result=true;
 }
 return result;
}

bool SDGF_Collision::check_collision(const SDGF_Box &first,const SDGF_Box &second)
{
 return this->check_horizontal_collision(first,second) || this->check_vertical_collision(first,second);
}
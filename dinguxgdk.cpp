/*
Dingux game development kit license

Copyright (C) 2015-2020 Popov Evgeniy Alekseyevich

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

#include "dinguxgdk.h"

namespace DINGUXGDK
{

void Halt(const char *message)
{
 puts(message);
 exit(EXIT_FAILURE);
}

Frame::Frame()
{
 frame_width=0;
 frame_height=0;
 pixels=0;
 length=0;
 buffer=NULL;
 shadow=NULL;
}

Frame::~Frame()
{
 if(buffer!=NULL)
 {
  free(buffer);
  buffer=NULL;
 }
 if(shadow!=NULL)
 {
  free(shadow);
  shadow=NULL;
 }

}

unsigned short int *Frame::create_buffer(const char *error)
{
 unsigned short int *target;
 pixels=(size_t)frame_width*(size_t)frame_height;
 target=(unsigned short int*)calloc(pixels,sizeof(unsigned short int));
 length=pixels*sizeof(unsigned short int);
 if(target==NULL)
 {
  Halt(error);
 }
 return target;
}

unsigned short int Frame::get_bgr565(const unsigned char red,const unsigned char green,const unsigned char blue)
{
 return (blue >> 3) +((green >> 2) << 5)+((red >> 3) << 11); // This code bases on code from SVGALib
}

size_t Frame::get_offset(const unsigned long int x,const unsigned long int y)
{
 return (size_t)x+(size_t)y*(size_t)frame_width;
}

void Frame::set_size(const unsigned long int surface_width,const unsigned long int surface_height)
{
 frame_width=surface_width;
 frame_height=surface_height;
}

void Frame::create_buffers()
{
 buffer=this->create_buffer("Can't allocate memory for render buffer");
 shadow=this->create_buffer("Can't allocate memory for shadow buffer");
}

unsigned short int *Frame::get_buffer()
{
 return buffer;
}

size_t Frame::get_length()
{
 return length;
}

void Frame::draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned char red,const unsigned char green,const unsigned char blue)
{
 if((x<frame_width)&&(y<frame_height))
 {
  buffer[this->get_offset(x,y)]=this->get_bgr565(red,green,blue);
 }

}

void Frame::clear_screen()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  buffer[index]=0;
 }

}

void Frame::save()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  shadow[index]=buffer[index];
 }

}

void Frame::restore()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  buffer[index]=shadow[index];
 }

}

void Frame::restore(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int target_x,target_y,stop_x,stop_y;
 size_t position;
 stop_x=x+width;
 stop_y=y+height;
 if ((x<frame_width)&&(y<frame_height))
 {
  if ((stop_x<=frame_width)&&(stop_y<=frame_height))
  {
   for (target_x=x;target_x<stop_x;++target_x)
   {
    for (target_y=y;target_y<stop_y;++target_y)
    {
     position=this->get_offset(target_x,target_y);
     buffer[position]=shadow[position];
    }

   }

  }

 }

}

FPS::FPS()
{
 start=time(NULL);
 current=0;
 fps=0;
}

FPS::~FPS()
{

}

void FPS::update_counter()
{
 time_t stop;
 if(current==0) start=time(NULL);
 ++current;
 stop=time(NULL);
 if(difftime(stop,start)>=1)
 {
  fps=current;
  current=0;
 }

}

unsigned long int FPS::get_fps()
{
 return fps;
}

Render::Render()
{
 start=0;
 device=open("/dev/fb0",O_RDWR);
 if(device==-1)
 {
  Halt("Can't get access to frame buffer");
 }
 memset(&setting,0,sizeof(FBIOGET_VSCREENINFO));
 memset(&configuration,0,sizeof(FBIOGET_FSCREENINFO));
}

Render::~Render()
{
 if(device!=-1) close(device);
}

void Render::read_base_configuration()
{
 if(ioctl(device,FBIOGET_VSCREENINFO,&setting)==-1)
 {
  Halt("Can't read framebuffer setting");
 }

}

void Render::read_advanced_configuration()
{
 if(ioctl(device,FBIOGET_FSCREENINFO,&configuration)==-1)
 {
  Halt("Can't read framebuffer setting");
 }

}

void Render::read_configuration()
{
 this->read_base_configuration();
 this->read_advanced_configuration();
}

void Render::get_start_offset()
{
 start=setting.xoffset*(setting.bits_per_pixel/CHAR_BIT)+setting.yoffset*configuration.line_length;
}

void Render::prepare_render()
{
 this->read_configuration();
 this->get_start_offset();
}

void Render::refresh()
{
 lseek(device,start,SEEK_SET);
 write(device,this->get_buffer(),this->get_length());
}

unsigned long int Render::get_width()
{
 return setting.xres;
}

unsigned long int Render::get_height()
{
 return setting.yres;
}

unsigned long int Render::get_color()
{
 return setting.bits_per_pixel;
}

Screen::Screen()
{

}

Screen::~Screen()
{

}

void Screen::initialize()
{
 this->prepare_render();
 this->set_size(this->get_width(),this->get_height());
 this->create_buffers();
}

void Screen::update()
{
 this->refresh();
 this->update_counter();
}

Screen* Screen::get_handle()
{
 return this;
}

Gamepad::Gamepad()
{
 current=NULL;
 preversion=NULL;
 device=-1;
 length=sizeof(input_event);
 memset(&input,0,length);
}

Gamepad::~Gamepad()
{
 if(device!=-1) close(device);
 if(current!=NULL) free(current);
 if(preversion!=NULL) free(preversion);
}

unsigned char *Gamepad::create_buffer()
{
 unsigned char *buffer;
 buffer=(unsigned char*)calloc(BUTTON_AMOUNT,sizeof(unsigned char));
 if (buffer==NULL)
 {
  Halt("Can't allocate memory for input buffer");
 }
 return buffer;
}

void Gamepad::create_buffers()
{
 current=this->create_buffer();
 preversion=this->create_buffer();
}

void Gamepad::open_device()
{
 device=open("/dev/event0",O_RDONLY|O_NONBLOCK|O_NOCTTY);
 if (device==-1)
 {
  Halt("Can't get access to gamepad");
 }

}

unsigned char Gamepad::get_state()
{
 unsigned char state;
 state=GAMEPAD_RELEASE;
 if (input.value!=GAMEPAD_RELEASE)
 {
  state=GAMEPAD_PRESS;
 }
 return state;
}

size_t Gamepad::get_button()
{
 size_t button;
 button=BUTTON_UP;
 switch (input.code)
 {
  case KEY_UP:
  button=BUTTON_UP;
  break;
  case KEY_DOWN:
  button=BUTTON_DOWN;
  break;
  case KEY_LEFT:
  button=BUTTON_LEFT;
  break;
  case KEY_RIGHT:
  button=BUTTON_RIGHT;
  break;
  case KEY_LEFTCTRL:
  button=BUTTON_A;
  break;
  case KEY_LEFTALT:
  button=BUTTON_B;
  break;
  case KEY_SPACE:
  button=BUTTON_X;
  break;
  case KEY_LEFTSHIFT:
  button=BUTTON_Y;
  break;
  case KEY_BACKSPACE:
  button=BUTTON_R;
  break;
  case KEY_TAB:
  button=BUTTON_L;
  break;
  case KEY_ENTER:
  button=BUTTON_START;
  break;
  case KEY_ESC:
  button=BUTTON_SELECT;
  break;
  case KEY_POWER:
  button=BUTTON_POWER;
  break;
  case KEY_PAUSE:
  button=BUTTON_HOLD;
  break;
  default:
  button=BUTTON_UP;
  break;
 }
 return button;
}

bool Gamepad::check_state(const size_t button,const unsigned char state)
{
 bool result;
 result=false;
 if (button<BUTTON_AMOUNT)
 {
  if (current[button]==state)
  {
   if(preversion[button]!=state) result=true;
  }

 }
 preversion[button]=current[button];
 return result;
}

void Gamepad::initialize()
{
 this->open_device();
 this->create_buffers();
}

void Gamepad::update()
{
 while (read(device,&input,length)>0)
 {
  if (input.type==EV_KEY)
  {
   current[this->get_button()]=this->get_state();
  }

 }

}

bool Gamepad::check_hold(const size_t button)
{
 bool result;
 result=false;
 if (button<BUTTON_AMOUNT)
 {
  if(current[button]!=GAMEPAD_RELEASE) result=true;
 }
 preversion[button]=current[button];
 return result;
}

bool Gamepad::check_press(const size_t button)
{
 return this->check_state(button,GAMEPAD_PRESS);
}

bool Gamepad::check_release(const size_t button)
{
 return this->check_state(button,GAMEPAD_RELEASE);
}

unsigned long int Memory::get_total_memory()
{
 unsigned long int memory;
 struct sysinfo information;
 memory=0;
 if (sysinfo(&information)==0) memory=information.totalram*information.mem_unit;
 return memory;
}

unsigned long int Memory::get_free_memory()
{
 unsigned long int memory;
 struct sysinfo information;
 memory=0;
 if (sysinfo(&information)==0) memory=information.freeram*information.mem_unit;
 return memory;
}

System::System()
{
 srand(time(NULL));
}

System::~System()
{

}

unsigned long int System::get_random(const unsigned long int number)
{
 return rand()%number;
}

void System::quit()
{
 exit(EXIT_SUCCESS);
}

void System::run(const char *command)
{
 system(command);
}

char* System::read_environment(const char *variable)
{
 return getenv(variable);
}

bool System::file_exist(const char *name)
{
 FILE *target;
 bool result;
 result=false;
 target=fopen(name,"rb");
 if (target!=NULL)
 {
  result=true;
  fclose(target);
 }
 return result;
}

bool System::delete_file(const char *name)
{
 bool result;
 result=false;
 if (remove(name)==0)
 {
  result=true;
 }
 return result;
}

void System::enable_logging(const char *name)
{
 if(freopen(name,"wt",stdout)==NULL)
 {
  Halt("Can't create log file");
 }

}

Binary_File::Binary_File()
{
 target=NULL;
}

Binary_File::~Binary_File()
{
 if(target!=NULL)
 {
  fclose(target);
  target=NULL;
 }

}

void Binary_File::open(const char *name,const char *mode)
{
 target=fopen(name,mode);
 if(target==NULL)
 {
  Halt("Can't open the binary file");
 }

}

void Binary_File::create_temp()
{
 target=tmpfile();
 if(target==NULL)
 {
  Halt("Can't create a temporary file");
 }

}

void Binary_File::open_read(const char *name)
{
 this->open(name,"rb");
}

void Binary_File::open_write(const char *name)
{
 this->open(name,"w+b");
}

void Binary_File::close()
{
 if(target!=NULL)
 {
  fclose(target);
  target=NULL;
 }

}

void Binary_File::set_position(const long int offset)
{
 fseek(target,offset,SEEK_SET);
}

long int Binary_File::get_position()
{
 return ftell(target);
}

long int Binary_File::get_length()
{
 long int result;
 fseek(target,0,SEEK_END);
 result=ftell(target);
 rewind(target);
 return result;
}

void Binary_File::read(void *buffer,const size_t length)
{
 fread(buffer,sizeof(char),length,target);
}

void Binary_File::write(void *buffer,const size_t length)
{
 fwrite(buffer,sizeof(char),length,target);
}

bool Binary_File::check_error()
{
 bool result;
 result=false;
 if(ferror(target)!=0) result=true;
 return result;
}

Timer::Timer()
{
 interval=0;
 start=time(NULL);
}

Timer::~Timer()
{

}

void Timer::set_timer(const unsigned long int seconds)
{
 interval=seconds;
 start=time(NULL);
}

bool Timer::check_timer()
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

Primitive::Primitive()
{
 surface=NULL;
 color.red=0;
 color.green=0;
 color.blue=0;
}

Primitive::~Primitive()
{
 color.red=0;
 color.green=0;
 color.blue=0;
 surface=NULL;
}

void Primitive::initialize(Screen *Screen)
{
 surface=Screen;
}

void Primitive::set_color(const unsigned char red,const unsigned char green,const unsigned char blue)
{
 color.red=red;
 color.green=green;
 color.blue=blue;
}

void Primitive::draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2)
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

void Primitive::draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 this->draw_line(x,y,stop_x,y);
 this->draw_line(x,stop_y,stop_x,stop_y);
 this->draw_line(x,y,x,stop_y);
 this->draw_line(stop_x,y,stop_x,stop_y);
}

void Primitive::draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
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

Image::Image()
{
 width=0;
 height=0;
 data=NULL;
}

Image::~Image()
{
 if(data!=NULL) free(data);
}

unsigned char *Image::create_buffer(const size_t length)
{
 unsigned char *result;
 result=(unsigned char*)calloc(length,sizeof(unsigned char));
 if(result==NULL)
 {
  Halt("Can't allocate memory for image buffer");
 }
 return result;
}

void Image::clear_buffer()
{
 if(data!=NULL)
 {
  free(data);
  data=NULL;
 }

}

void Image::load_tga(const char *name)
{
 Binary_File target;
 size_t index,position,amount,compressed_length,uncompressed_length;
 unsigned char *compressed;
 unsigned char *uncompressed;
 TGA_head head;
 TGA_map color_map;
 TGA_image image;
 this->clear_buffer();
 target.open_read(name);
 compressed_length=(size_t)target.get_length()-18;
 target.read(&head,3);
 target.read(&color_map,5);
 target.read(&image,10);
 if((head.color_map!=0)||(image.color!=24))
 {
  Halt("Invalid image format");
 }
 if(head.type!=2)
 {
  if(head.type!=10)
  {
   Halt("Invalid image format");
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
  target.read(uncompressed,uncompressed_length);
 }
 if(head.type==10)
 {
  compressed=this->create_buffer(compressed_length);
  target.read(compressed,compressed_length);
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
 target.close();
 data=uncompressed;
}

void Image::load_pcx(const char *name)
{
 Binary_File target;
 unsigned long int x,y;
 size_t index,position,line,row,length,uncompressed_length;
 unsigned char repeat;
 unsigned char *original;
 unsigned char *uncompressed;
 PCX_head head;
 this->clear_buffer();
 target.open_read(name);
 length=(size_t)target.get_length()-128;
 target.read(&head,128);
 if((head.color*head.planes!=24)&&(head.compress!=1))
 {
  Halt("Incorrect image format");
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
 target.read(original,length);
 target.close();
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

unsigned long int Image::get_width()
{
 return width;
}

unsigned long int Image::get_height()
{
 return height;
}

size_t Image::get_data_length()
{
 return (size_t)width*(size_t)height*3;
}

unsigned char *Image::get_data()
{
 return data;
}

void Image::destroy_image()
{
 width=0;
 height=0;
 this->clear_buffer();
}

Surface::Surface()
{
 width=0;
 height=0;
 image=NULL;
 surface=NULL;
}

Surface::~Surface()
{
 surface=NULL;
 if(image!=NULL) free(image);
}

IMG_Pixel *Surface::create_buffer(const unsigned long int image_width,const unsigned long int image_height)
{
 IMG_Pixel *result;
 size_t length;
 length=(size_t)image_width*(size_t)image_height;
 result=(IMG_Pixel*)calloc(length,3);
 if(result==NULL)
 {
  Halt("Can't allocate memory for image buffer");
 }
 return result;
}

void Surface::save()
{
 surface->save();
}

void Surface::restore()
{
 surface->restore();
}

void Surface::clear_buffer()
{
 if(image!=NULL)
 {
  free(image);
  image=NULL;
 }

}

void Surface::load_from_buffer(Image &buffer)
{
 width=buffer.get_width();
 height=buffer.get_height();
 this->clear_buffer();
 image=this->create_buffer(width,height);
 memmove(image,buffer.get_data(),buffer.get_data_length());
}

void Surface::set_width(const unsigned long int image_width)
{
 width=image_width;
}

void Surface::set_height(const unsigned long int image_height)
{
 height=image_height;
}

void Surface::set_buffer(IMG_Pixel *buffer)
{
 image=buffer;
}

size_t Surface::get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y)
{
 return (size_t)start+(size_t)x+(size_t)y*(size_t)width;
}

void Surface::draw_image_pixel(const size_t offset,const unsigned long int x,const unsigned long int y)
{
 surface->draw_pixel(x,y,image[offset].red,image[offset].green,image[offset].blue);
}

bool Surface::compare_pixels(const size_t first,const size_t second)
{
 bool result;
 result=false;
 if ((image[first].red!=image[second].red)||(image[first].green!=image[second].green))
 {
  result=true;
 }
 else
 {
  if(image[first].blue!=image[second].blue) result=true;
 }
 return result;
}

void Surface::initialize(Screen *screen)
{
 surface=screen;
}

size_t Surface::get_length()
{
 return (size_t)width*(size_t)height;
}

IMG_Pixel *Surface::get_image()
{
 return image;
}

unsigned long int Surface::get_image_width()
{
 return width;
}

unsigned long int Surface::get_image_height()
{
 return height;
}

void Surface::mirror_image(const MIRROR_TYPE kind)
{
 unsigned long int x,y;
 size_t index,index2;
 IMG_Pixel *mirrored_image;
 mirrored_image=this->create_buffer(width,height);
 if (kind==MIRROR_HORIZONTAL)
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
 if(kind==MIRROR_VERTICAL)
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

void Surface::resize_image(const unsigned long int new_width,const unsigned long int new_height)
{
 float x_ratio,y_ratio;
 unsigned long int x,y;
 size_t index,index2;
 IMG_Pixel *scaled_image;
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

Canvas::Canvas()
{
 start=0;
 frame=1;
 frames=1;
}

Canvas::~Canvas()
{

}

void Canvas::set_frame(const unsigned long int target)
{
 if (target<=frames)
 {
  frame=target;
 }

}

void Canvas::increase_frame()
{
 ++frame;
 if (frame>frames)
 {
  frame=1;
 }

}

void Canvas::set_frames(const unsigned long int amount)
{
 if(amount>1) frames=amount;
}

unsigned long int Canvas::get_frames()
{
 return frames;
}

unsigned long int Canvas::get_frame()
{
 return frame;
}

void Canvas::load_image(Image &buffer)
{
 this->load_from_buffer(buffer);
}

Background::Background()
{
 background_width=0;
 background_height=0;
 current=0;
 current_kind=NORMAL_BACKGROUND;
}

Background::~Background()
{

}

void Background::draw_background_pixel(const unsigned long int x,const unsigned long int y)
{
 size_t offset;
 offset=this->get_offset(start,x,y);
 this->draw_image_pixel(offset,x,y);
}

void Background::slow_draw_background()
{
 unsigned long int x,y;
 for(x=0;x<background_width;++x)
 {
  for(y=0;y<background_height;++y)
  {
   this->draw_background_pixel(x,y);
  }

 }

}

unsigned long int Background::get_background_width()
{
 return background_width;
}

unsigned long int Background::get_background_height()
{
 return background_height;
}

void Background::set_kind(const BACKGROUND_TYPE kind)
{
 switch(kind)
 {
  case NORMAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height();
  start=0;
  break;
  case HORIZONTAL_BACKGROUND:
  background_width=this->get_image_width()/this->get_frames();
  background_height=this->get_image_height();
  start=(this->get_frame()-1)*background_width;
  break;
  case VERTICAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height()/this->get_frames();
  start=(this->get_frame()-1)*background_width*background_height;
  break;
 }
 current_kind=kind;
}

void Background::set_target(const unsigned long int target)
{
 this->set_frame(target);
 this->set_kind(current_kind);
}

void Background::step()
{
 this->increase_frame();
 this->set_kind(current_kind);
}

void Background::draw_background()
{
 if (current!=this->get_frame())
 {
  this->slow_draw_background();
  this->save();
  current=this->get_frame();
 }
 else
 {
  this->restore();
 }

}

Sprite::Sprite()
{
 transparent=true;
 current_x=0;
 current_y=0;
 sprite_width=0;
 sprite_height=0;
 current_kind=SINGLE_SPRITE;
}

Sprite::~Sprite()
{

}

void Sprite::draw_sprite_pixel(const size_t offset,const unsigned long int x,const unsigned long int y)
{
 if (transparent==true)
 {
  if(this->compare_pixels(0,offset)==true) this->draw_image_pixel(offset,x,y);
 }
 else
 {
  this->draw_image_pixel(offset,x,y);
 }

}

void Sprite::set_transparent(const bool enabled)
{
 transparent=enabled;
}

bool Sprite::get_transparent()
{
 return transparent;
}

void Sprite::set_x(const unsigned long int x)
{
 current_x=x;
}

void Sprite::set_y(const unsigned long int y)
{
 current_y=y;
}

unsigned long int Sprite::get_x()
{
 return current_x;
}

unsigned long int Sprite::get_y()
{
 return current_y;
}

unsigned long int Sprite::get_width()
{
 return sprite_width;
}

unsigned long int Sprite::get_height()
{
 return sprite_height;
}

Sprite* Sprite::get_handle()
{
 return this;
}

Collision_Box Sprite::get_box()
{
 Collision_Box target;
 target.x=current_x;
 target.y=current_y;
 target.width=sprite_width;
 target.height=sprite_height;
 return target;
}

void Sprite::set_kind(const SPRITE_TYPE kind)
{
 switch(kind)
 {
  case SINGLE_SPRITE:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height();
  start=0;
  break;
  case HORIZONTAL_STRIP:
  sprite_width=this->get_image_width()/this->get_frames();
  sprite_height=this->get_image_height();
  start=(this->get_frame()-1)*sprite_width;
  break;
  case VERTICAL_STRIP:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height()/this->get_frames();
  start=(this->get_frame()-1)*sprite_width*sprite_height;
  break;
 }
 current_kind=kind;
}

SPRITE_TYPE Sprite::get_kind()
{
 return current_kind;
}

void Sprite::set_target(const unsigned long int target)
{
 this->set_frame(target);
 this->set_kind(current_kind);
}

void Sprite::step()
{
 this->increase_frame();
 this->set_kind(current_kind);
}

void Sprite::set_position(const unsigned long int x,const unsigned long int y)
{
 current_x=x;
 current_y=y;
}

void Sprite::clone(Sprite &target)
{
 this->set_width(target.get_image_width());
 this->set_height(target.get_image_height());
 this->set_frames(target.get_frames());
 this->set_kind(target.get_kind());
 this->set_transparent(target.get_transparent());
 this->set_buffer(this->create_buffer(target.get_image_width(),target.get_image_width()));
 memmove(this->get_image(),target.get_image(),target.get_length());
}

void Sprite::draw_sprite()
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

void Sprite::draw_sprite(const unsigned long int x,const unsigned long int y)
{
 this->set_position(x,y);
 this->draw_sprite();
}

Tileset::Tileset()
{
 offset=0;
 rows=0;
 columns=0;
 tile_width=0;
 tile_height=0;
}

Tileset::~Tileset()
{

}

unsigned long int Tileset::get_tile_width()
{
 return tile_width;
}

unsigned long int Tileset::get_tile_height()
{
 return tile_height;
}

unsigned long int Tileset::get_rows()
{
 return rows;
}

unsigned long int Tileset::get_columns()
{
 return columns;
}

void Tileset::select_tile(const unsigned long int row,const unsigned long int column)
{
 if ((row<rows)&&(column<columns))
 {
  offset=this->get_offset(0,row*tile_width,column*tile_height);
 }

}

void Tileset::draw_tile(const unsigned long int x,const unsigned long int y)
{
 size_t tile_offset;
 unsigned long int tile_x,tile_y;
 for(tile_x=0;tile_x<tile_width;++tile_x)
 {
  for(tile_y=0;tile_y<tile_height;++tile_y)
  {
   tile_offset=offset+this->get_offset(0,tile_x,tile_y);
   this->draw_image_pixel(tile_offset,x+tile_x,y+tile_y);
  }

 }

}

void Tileset::load_tileset(Image &buffer,const unsigned long int row_amount,const unsigned long int column_amount)
{
 if ((row_amount>0)&&(column_amount>0))
 {
  this->load_from_buffer(buffer);
  rows=row_amount;
  columns=column_amount;
  tile_width=this->get_image_width()/rows;
  tile_height=this->get_image_height()/columns;
 }

}

Text::Text()
{
 current_x=0;
 current_y=0;
 font=NULL;
}

Text::~Text()
{
 font=NULL;
}

void Text::increase_position()
{
 unsigned long int x;
 x=font->get_x()+font->get_width();
 font->set_x(x);
}

void Text::set_position(const unsigned long int x,const unsigned long int y)
{
 font->set_position(x,y);
 current_x=x;
 current_y=y;
}

void Text::load_font(Sprite *target)
{
 font=target;
 font->set_frames(128);
 font->set_kind(HORIZONTAL_STRIP);
}

void Text::draw_character(const char target)
{
 font->set_target((unsigned char)target+1);
 font->draw_sprite();
}

void Text::draw_text(const char *text)
{
 size_t index,length;
 length=strlen(text);
 font->set_position(current_x,current_y);
 for (index=0;index<length;++index)
 {
  if (text[index]<32) continue;
  this->draw_character(text[index]);
  this->increase_position();
 }
 font->set_position(current_x,current_y);
}

Collision_Box Collision::generate_box(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 Collision_Box result;
 result.x=x;
 result.y=y;
 result.width=width;
 result.height=height;
 return result;
}

bool Collision::check_horizontal_collision(const Collision_Box &first,const Collision_Box &second)
{
 bool result;
 result=false;
 if((first.x+first.width)>=second.x)
 {
  if(first.x<=(second.x+second.width)) result=true;
 }
 return result;
}

bool Collision::check_vertical_collision(const Collision_Box &first,const Collision_Box &second)
{
 bool result;
 result=false;
 if((first.y+first.height)>=second.y)
 {
  if(first.y<=(second.y+second.height)) result=true;
 }
 return result;
}

bool Collision::check_collision(const Collision_Box &first,const Collision_Box &second)
{
 return this->check_horizontal_collision(first,second) || this->check_vertical_collision(first,second);
}

}
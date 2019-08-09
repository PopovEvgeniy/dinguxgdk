#include "SDGF.h"

int main(void)
{
 long int x,y,screen_width,screen_height;
 char perfomance[8];
 SDGF::Screen screen;
 SDGF::Gamepad gamepad;
 SDGF::Timer timer;
 SDGF::Image image;
 SDGF::Background space;
 SDGF::Sprite ship,font;
 SDGF::Text text;
 screen.initialize();
 screen_width=screen.get_width();
 screen_height=screen.get_height();
 x=screen_width/2;
 y=screen_height/2;
 image.load_tga("space.tga");
 space.load_image(image);
 image.load_tga("ship.tga");
 ship.load_image(image);
 image.load_tga("font.tga");
 font.load_image(image);
 text.load_font(font.get_handle());
 gamepad.initialize();
 space.initialize(screen.get_handle());
 ship.initialize(screen.get_handle());
 font.initialize(screen.get_handle());
 space.resize_image(screen_width,screen_height);
 space.set_kind(NORMAL_BACKGROUND);
 screen.clear_screen();
 ship.set_frames(2);
 ship.set_kind(HORIZONTAL_STRIP);
 text.set_position(font.get_width(),font.get_height());
 timer.set_timer(1);
 memset(perfomance,0,8);
 while(1)
 {
  screen.update();
  gamepad.update();
  if(gamepad.get_press()==BUTTON_X) break;
  if(gamepad.get_press()==BUTTON_A) ship.mirror_image(MIRROR_HORIZONTAL);
  if(gamepad.get_press()==BUTTON_B) ship.mirror_image(MIRROR_VERTICAL);
  if(gamepad.get_hold()==BUTTON_UP) y-=4;
  if(gamepad.get_hold()==BUTTON_DOWN) y+=4;
  if(gamepad.get_hold()==BUTTON_LEFT) x-=4;
  if(gamepad.get_hold()==BUTTON_RIGHT) x+=4;
  if((x<=0)||(x>=screen_width)) x=screen_width/2;
  if((y<=0)||(y>=screen_height)) y=screen_height/2;
  sprintf(perfomance,"%ld",screen.get_fps());
  space.draw_background();
  text.draw_text(perfomance);
  ship.set_position(x,y);
  ship.draw_sprite();
  if (timer.check_timer()==true)
  {
   ship.step();
  }

 }
 return 0;
}
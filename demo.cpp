#include "SDGF.h"

int main(void)
{
 long int x,y,screen_width,screen_height;
 unsigned char frame,scroll_frame;
 SDGF_Screen screen;
 SDGF_System System;
 SDGF_Gamepad gamepad;
 SDGF_Sound sound;
 SDGF_Audio audio;
 SDGF_Player player;
 SDGF_Timer timer,scroll_timer;
 SDGF_Image image;
 SDGF_Background space;
 SDGF_Sprite ship,font;
 SDGF_Text text;
 screen_width=screen.get_width();
 screen_height=screen.get_height();
 x=screen_width/2;
 y=screen_height/2;
 System.enable_logging("log.txt");
 image.load_tga("space.tga");
 space.load_image(image);
 image.load_tga("ship.tga");
 ship.load_image(image);
 image.load_tga("font.tga");
 font.load_image(image);
 text.load_font(font.get_handle());
 space.initialize(screen.get_handle());
 ship.initialize(screen.get_handle());
 font.initialize(screen.get_handle());
 space.set_frames(3);
 space.resize_image(screen_width*space.get_frames(),screen_height);
 screen.clear_screen();
 frame=1;
 scroll_frame=1;
 ship.set_frames(2);
 text.set_position(font.get_sprite_width(),font.get_sprite_height());
 timer.set_timer(1);
 scroll_timer.set_timer(2);
 sound.set_volume(255);
 player.initialize(sound.get_handle());
 audio.load_wave("space.wav",player);
 while(1)
 {
  if(player.play()==false) player.rewind_audio();
  gamepad.update();
  if(gamepad.get_press()==SDGF_KEY_X) break;
  if(gamepad.get_press()==SDGF_KEY_A) ship.mirror_image(0);
  if(gamepad.get_press()==SDGF_KEY_B) ship.mirror_image(1);
  if(gamepad.get_press()==SDGF_KEY_UP) y-=4;
  if(gamepad.get_press()==SDGF_KEY_DOWN) y+=4;
  if(gamepad.get_press()==SDGF_KEY_LEFT) x-=4;
  if(gamepad.get_press()==SDGF_KEY_RIGHT) x+=4;
  if((x<=0)||(x>=screen_width)) x=screen_width/2;
  if((y<=0)||(y>=screen_height)) y=screen_height/2;
  space.draw_horizontal_background(scroll_frame);
  text.draw_text("Technical demonstration #5");
  ship.draw_sprite_frame(x,y,frame);
  screen.refresh();
  if (timer.check_timer()==true)
  {
   frame++;
   if (frame>ship.get_frames()) frame=1;
  }
  if (scroll_timer.check_timer()==true)
  {
   scroll_frame++;
   if (scroll_frame>space.get_frames()) scroll_frame=1;
  }

 }
 return 0;
}
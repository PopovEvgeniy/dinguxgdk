#include "SDGF.h"

int main(void)
{
 long int x,y,screen_width,screen_height;
 unsigned char frame;
 SDGF_Screen screen;
 SDGF_Gamepad gamepad;
 SDGF_Sound sound;
 SDGF_Audio audio;
 SDGF_Player player;
 SDGF_Timer timer;
 SDGF_Image image;
 SDGF_Canvas space,ship,font;
 screen_width=screen.get_width();
 screen_height=screen.get_height();
 x=screen_width/2;
 y=screen_height/2;
 image.load_pcx("space.pcx",space);
 image.load_pcx("ship.pcx",ship);
 image.load_pcx("font.pcx",font);
 space.initialize(screen.get_handle());
 ship.initialize(screen.get_handle());
 font.initialize(screen.get_handle());
 space.resize_image(screen_width,screen_height);
 screen.clear_screen();
 frame=1;
 timer.set_timer(1);
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
  space.draw_background();
  font.draw_text(screen_width/2,font.get_height(),"Technical demonstration #4");
  ship.draw_sprite_frame(x,y,frame,2);
  screen.refresh();
  if (timer.check_timer()==true)
  {
   frame++;
   if (frame>2) frame=1;
  }

 }
 return 0;
}
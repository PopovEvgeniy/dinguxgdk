#include "dinguxgdk.h"

int main(void)
{
 char level[6];
 DINGUXGDK::Screen screen;
 DINGUXGDK::Gamepad gamepad;
 DINGUXGDK::Image image;
 DINGUXGDK::Sprite font;
 DINGUXGDK::Battery battery;
 DINGUXGDK::Text text;
 memset(level,0,6);
 battery.initialize(A320);
 gamepad.initialize();
 screen.initialize();
 font.initialize(screen.get_handle());
 image.load_tga("font.tga");
 font.load_image(image);
 text.load_font(font.get_handle());
 text.set_position(screen.get_width()/2,screen.get_height()/2);
 while(1)
 {
  screen.update();
  gamepad.update();
  if(gamepad.check_hold(BUTTON_START)==true) break;
  sprintf(level,"%hu",battery.get_level());
  text.draw_text(level);
 }
 return 0;
}
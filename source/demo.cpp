#include "dinguxgdk.h"

int main()
{
 char perfomance[8];
 DINGUXGDK::Backlight light;
 DINGUXGDK::Screen screen;
 DINGUXGDK::Gamepad gamepad;
 DINGUXGDK::Sound sound;
 DINGUXGDK::Audio audio;
 DINGUXGDK::Player player;
 DINGUXGDK::Timer timer;
 DINGUXGDK::Image image;
 DINGUXGDK::Background space;
 DINGUXGDK::Sprite ship,font;
 DINGUXGDK::Text text;
 screen.initialize();
 image.load_tga("space.tga");
 space.load_image(image);
 image.load_tga("ship.tga");
 ship.load_sprite(image,HORIZONTAL_STRIP,2);
 ship.set_position(screen.get_width()/2,screen.get_height()/2);
 image.load_tga("font.tga");
 font.load_image(image);
 text.load_font(font.get_handle());
 gamepad.initialize();
 space.initialize(screen.get_handle());
 ship.initialize(screen.get_handle());
 font.initialize(screen.get_handle());
 space.resize_image(screen.get_width(),screen.get_height());
 space.set_kind(NORMAL_BACKGROUND);
 screen.clear_screen();
 text.set_position(font.get_width(),font.get_height());
 timer.set_timer(1);
 sound.initialize(44100);
 player.initialize(sound.get_handle());
 audio.load_wave("space.wav");
 player.load(audio.get_handle());
 light.set_light(light.get_minimum());
 memset(perfomance,0,8);
 while(1)
 {
  screen.update();
  gamepad.update();
  player.loop();
  if (gamepad.check_press(BUTTON_START)==true) break;
  if (gamepad.check_press(BUTTON_A)==true) ship.horizontal_mirror();
  if (gamepad.check_press(BUTTON_B)==true) ship.vertical_mirror();
  if (gamepad.check_press(BUTTON_R)==true) light.turn_off();
  if (gamepad.check_press(BUTTON_L)==true) light.turn_on();
  if (gamepad.check_press(BUTTON_X)==true) light.increase_level();
  if (gamepad.check_press(BUTTON_Y)==true) light.decrease_level();
  if (gamepad.check_hold(BUTTON_UP)==true) ship.decrease_y(4);
  if (gamepad.check_hold(BUTTON_DOWN)==true) ship.increase_y(4);
  if (gamepad.check_hold(BUTTON_LEFT)==true) ship.decrease_x(4);
  if (gamepad.check_hold(BUTTON_RIGHT)==true) ship.increase_x(4);
  if (ship.get_x()>screen.get_width()) ship.set_x(screen.get_width()/2);
  if (ship.get_y()>screen.get_height()) ship.set_y(screen.get_height()/2);
  sprintf(perfomance,"%lu",screen.get_fps());
  space.draw_background();
  text.draw_text(perfomance);
  ship.draw_sprite();
  if (timer.check_timer()==true)
  {
   ship.step();
  }

 }
 return 0;
}
#include "SDGF.h"

#define MAP_WIDTH 16
#define MAP_HEIGHT 16

unsigned char MAP_ROWS[MAP_WIDTH]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
unsigned char MAP_COLUMNS[MAP_HEIGHT]={2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

SDGF::Tileset tilemap;

void draw_map()
{
 unsigned long int x,y;
 size_t row,column;
 x=0;
 y=0;
 for (row=0;row<MAP_WIDTH;++row)
 {
  for (column=0;column<MAP_HEIGHT;++column)
  {
   x=(unsigned long int)row*tilemap.get_tile_width();
   y=(unsigned long int)column*tilemap.get_tile_height();
   tilemap.select_tile(MAP_ROWS[row],MAP_COLUMNS[column]);
   tilemap.draw_tile(x,y);
  }

 }

}

int main(void)
{
 SDGF::Screen screen;
 SDGF::Sprite font;
 SDGF::Text text;
 SDGF::Image image;
 SDGF::Gamepad gamepad;
 font.initialize(screen.get_handle());
 tilemap.initialize(screen.get_handle());
 gamepad.initialize();
 image.load_tga("font.tga");
 font.load_image(image);
 text.load_font(font.get_handle());
 text.set_position(0,0);
 image.load_tga("grass.tga");
 tilemap.load_tileset(image,6,3);
 screen.initialize();
 draw_map();
 screen.save();
 while (true)
 {
  gamepad.update();
  if (gamepad.get_hold()==BUTTON_A) break;
  screen.restore();
  text.draw_text("Press A to exit");
  screen.update();
 }
 return 0;
}
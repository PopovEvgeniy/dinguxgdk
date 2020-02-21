flags=-g0 -O0

library:
	@mipsel-linux-uclibc-g++ -c dinguxgdk.cpp $(flags) -o dinguxgdk.o
	@mipsel-linux-uclibc-ar -r dinguxgdk.a dinguxgdk.o
	@rm *.o
demo:
	@mipsel-linux-uclibc-g++ demo.cpp dinguxgdk.a $(flags) -o demo_a320.dge
tileset:
	@mipsel-linux-uclibc-g++ tileset.cpp dinguxgdk.a $(flags) -o tileset_a320.dge
battery:
	@mipsel-linux-uclibc-g++ battery_level.cpp dinguxgdk.a $(flags) -o battery_a320.dge
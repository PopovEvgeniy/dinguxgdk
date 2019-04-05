flags=-g0 -O0

library:
	@mipsel-linux-uclibc-g++ -c SDGF.cpp $(flags) -o SDGF.o
	@mipsel-linux-uclibc-ar -r SDGF.a SDGF.o
	@rm *.o
demo:
	@mipsel-linux-uclibc-g++ demo.cpp SDGF.a $(flags) -o demo_a320.dge
tileset:
	@mipsel-linux-uclibc-g++ tileset.cpp SDGF.a $(flags) -o tileset_a320.dge
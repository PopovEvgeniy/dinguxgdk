flags=-G0 -O0

library:
	@mipsel-linux-uclibc-g++ -c SDGF.cpp $(flags) -o SDGF.o
	@mipsel-linux-uclibc-ar -r SDGF.a SDGF.o
	@rm *.o
program:
	@mipsel-linux-uclibc-g++ demo.cpp SDGF.a $(flags) -o demo_a320.dge
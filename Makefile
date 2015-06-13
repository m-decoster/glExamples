CC=g++
CFLAGS=-Wall
INCLUDES=-Isrc/libs/
LIBS=-Llib -lglfw3 -lSOIL -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
COMMON=src/examples/common/util.cpp src/examples/common/shader.cpp

all: hello_triangle hello_sprite hello_cube hello_heightmap

hello_triangle:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/01-hello_triangle/main.cpp $(COMMON) -o bin/01-hello_triangle.out $(LIBS)

hello_sprite:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/02-hello_sprite/main.cpp $(COMMON) -o bin/02-hello_sprite.out $(LIBS)
	cp src/examples/02-hello_sprite/image.png bin/image.png

hello_cube:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/03-hello_cube/main.cpp $(COMMON) -o bin/03-hello_cube.out $(LIBS)
	cp src/examples/03-hello_cube/image.png bin/image.png

hello_heightmap:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/04-hello_heightmap/main.cpp src/examples/04-hello_heightmap/heightmap.cpp $(COMMON) -o bin/04-hello_heightmap.out $(LIBS)
	cp src/examples/04-hello_heightmap/heightmap.bmp bin/heightmap.bmp

clean:
	rm bin/*

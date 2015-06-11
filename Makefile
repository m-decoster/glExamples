CC=g++
CFLAGS=-Wall
INCLUDES=-Isrc/libs/
LIBS=-Llib -lglfw3 -lSOIL -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all: hello_triangle hello_sprite

hello_triangle:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/01-hello_triangle/main.cpp src/examples/common/util.cpp src/examples/common/shader.cpp -o bin/01-hello_triangle.out $(LIBS)

hello_sprite:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/02-hello_sprite/main.cpp src/examples/common/util.cpp src/examples/common/shader.cpp -o bin/02-hello_sprite.out $(LIBS)
	cp src/examples/02-hello_sprite/image.png bin/image.png

clean:
	rm bin/*

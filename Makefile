CC=g++
CFLAGS=-Wall
INCLUDES=-Isrc/libs/
LIBS=-Llib -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

hello_triangle:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/01-hello_triangle/main.cpp src/examples/common/util.cpp src/examples/common/shader.cpp -o bin/01-hello_triangle.out $(LIBS)

clean:
	rm bin/*

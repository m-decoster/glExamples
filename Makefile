CC=g++
CFLAGS=-Wall
INCLUDES=-Isrc/libs/
LIBS=-Llib -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

hello_triangle:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/01-hello_triangle/main.cpp -o bin/01-hello_triangle $(LIBS)

clean:
	rm bin/*

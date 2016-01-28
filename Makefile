CC=g++
CFLAGS=-Wall -std=c++11
INCLUDES=-Isrc/libs/
LIBS=-Llib -lglfw3 -lSOIL -lassimp -lz -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
COMMON=src/examples/common/util.cpp src/examples/common/shader.cpp src/examples/common/camera.cpp

all: hello_triangle hello_sprite hello_cube hello_heightmap hello_mesh render_to_texture cubemaps instancing particles sprite_batching morph_target_animation uniform_buffer_objects forward_rendering shadows billboards deferred_shading transparency hdr point_shadows

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

hello_mesh:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/05-hello_mesh/main.cpp src/examples/05-hello_mesh/mesh.cpp src/examples/05-hello_mesh/material.cpp $(COMMON) -o bin/05-hello_mesh.out $(LIBS)
	cp src/examples/05-hello_mesh/image.png bin/image.png
	cp src/examples/05-hello_mesh/test_mesh.obj bin/test_mesh.obj

render_to_texture:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/06-render_to_texture/main.cpp src/examples/06-render_to_texture/mesh.cpp src/examples/06-render_to_texture/material.cpp $(COMMON) -o bin/06-render_to_texture.out $(LIBS)
	cp src/examples/06-render_to_texture/image.png bin/image.png
	cp src/examples/06-render_to_texture/test_mesh.obj bin/test_mesh.obj

cubemaps:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/07-cubemaps/main.cpp src/examples/07-cubemaps/skybox.cpp $(COMMON) -o bin/07-cubemaps.out $(LIBS)
	cp src/examples/07-cubemaps/*.png bin/

instancing:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/08-instancing/main.cpp src/examples/08-instancing/mesh.cpp src/examples/08-instancing/material.cpp src/examples/08-instancing/skybox.cpp $(COMMON) -o bin/08-instancing.out $(LIBS)
	cp src/examples/08-instancing/asteroid.obj bin/asteroid.obj
	cp src/examples/08-instancing/*.png bin/

particles:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/09-particles/main.cpp src/examples/09-particles/particle.cpp $(COMMON) -o bin/09-particles.out $(LIBS)

sprite_batching:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/10-sprite_batching/main.cpp src/examples/10-sprite_batching/sprite.cpp src/examples/10-sprite_batching/spritebatcher.cpp $(COMMON) -o bin/10-sprite_batching.out $(LIBS)
	cp src/examples/10-sprite_batching/spritesheet.png bin/spritesheet.png

morph_target_animation:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/11-morph_target_animation/main.cpp $(COMMON) -o bin/11-morph_target_animation.out $(LIBS)

uniform_buffer_objects:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/12-uniform_buffer_objects/main.cpp $(COMMON) -o bin/12-uniform_buffer_objects.out $(LIBS)
	cp src/examples/12-uniform_buffer_objects/image.png bin/image.png

forward_rendering:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/13-forward_rendering/main.cpp $(COMMON) -o bin/13-forward_rendering.out $(LIBS)
	cp src/examples/13-forward_rendering/*.png bin/

shadows:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/14-shadows/main.cpp $(COMMON) -o bin/14-shadows.out $(LIBS)
	cp src/examples/14-shadows/*.png bin/

billboards:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/15-billboards/main.cpp $(COMMON) -o bin/15-billboards.out $(LIBS)
	cp src/examples/15-billboards/*.png bin/

deferred_shading:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/16-deferred_shading/main.cpp src/examples/16-deferred_shading/mesh.cpp $(COMMON) -o bin/16-deferred_shading.out $(LIBS)
	cp src/examples/16-deferred_shading/*.png bin/
	cp src/examples/16-deferred_shading/*.obj bin/

transparency:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/17-transparency/main.cpp $(COMMON) -o bin/17-transparency.out $(LIBS)
	cp src/examples/17-transparency/*.png bin/

hdr:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/18-hdr/main.cpp src/examples/18-hdr/mesh.cpp $(COMMON) -o bin/18-hdr.out $(LIBS)
	cp src/examples/18-hdr/*.png bin/
	cp src/examples/18-hdr/*.obj bin/

additive_lights:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/19-additive_lights/main.cpp $(COMMON) -o bin/19-additive_lights.out $(LIBS)

point_shadows:
	$(CC) $(INCLUDES) $(CFLAGS) src/examples/20-point_shadows/main.cpp src/examples/20-point_shadows/shader.cpp src/examples/20-point_shadows/util.cpp $(COMMON) -o bin/20-point_shadows.out $(LIBS)

clean:
	rm bin/*

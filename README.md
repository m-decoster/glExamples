# glExamples

This repository contains some OpenGL examples.
These examples have been written in C++ and use OpenGL 3.3.

**These are not tutorials**. While I have tried to document the code
as much as possible, the why and how is not explained. If you want a
tutorial on OpenGL, I suggest you try out [open.gl](http://open.gl) or
[learn opengl](http://learnopengl.com).  
The point of these examples is not to teach you how 3D rendering works.
These examples merely serve as a point of reference as to how you can
implement solutions to typical problems in OpenGL 3.3.

Each example resides in its own folder. Code that is often reused, such as
code for compiling and linking shader programs, is put into a seperate folder and reused in the different examples.

This code has been written with OS X as a target. No special care has been
taken to make sure the code is cross-platform. While the examples might
not compile out of the box on your machine, the code can still be reused in
your projects. You should be able to just plug it into an OpenGL application
that compiles and runs on your machine.

## Dependencies

These examples have some dependencies:

* [GLFW](http://www.glfw.org)
* [SOIL](http://www.lonesock.net/soil.html)
* [GLM](http://glm.g-truc.net)

## Compiling

### OS X

Make sure that you have compiled the GLFW dependency for your system as a
static library. You should now have a file called `libglfw3.a`. Place this
in a subdirectory `lib` of the root folder (on the same level as this `README.md` file).

Download the latest release of SOIL. You can try putting the binary in the `lib` folder, but this
will most likely fail if your computer is from the last few years. You're best off using the supplied
makefile in the `projects/makefile` folder. Note that you need to alter a line if your OS is 64 bit:
change the line that says `CXX = gcc` to `CXX = gcc -arch x86_64`. Run `make` and you will find the new
`libSOIL.a` file in the `lib` subdirectory of the directory where you downloaded SOIL to.

Now you can use the command `make EXAMPLE_NAME` to compile an example. The
resulting binaries will be placed in the `bin` folder.

## License

These examples are available under the MIT License. This is because public
domain is not recognized in every country. For more information, read
[this question and answer](http://programmers.stackexchange.com/questions/147111/what-is-wrong-with-the-unlicense).

## Examples

To compile all examples, you can run `make` or `make all`.

1. [Hello Triangle](#hello-triangle)
2. [Hello Sprite](#hello-sprite)
3. [Hello Cube](#hello-cube)
4. [Hello Heightmap](#hello-heightmap)

### Hello Triangle

**Compile**: `make hello_triangle`  
**Run**: `cd bin; ./01-hello-triangle.out`

This example shows the minimal code needed to render a simple colored triangle.

[Code](src/examples/01-hello_triangle)

![hello triangle](img/01-hello_triangle.tiff)

### Hello Sprite

**Compile**: `make hello_sprite`  
**Run**: `cd bin; ./02-hello_sprite.out`

This example shows the minimal code needed to render a textured quad. It also uses
`glDrawElements` to reduce duplication of vertices.

[Code](src/examples/02-hello_sprite)

![hello sprite](img/02-hello_sprite.tiff)

### Hello Cube

**Compile**: `make hello_cube`  
**Run**: `cd bin; ./03-hello_cube.out`

This example introduces 3D transformations and perspective projection. It also
uses depth testing to make sure the cube is rendered correctly.

[Code](src/examples/03-hello_cube)

![hello cube](img/03-hello_cube.tiff)

### Hello Heightmap

**Compile**: `make hello_heightmap`  
**Run**: `cd bin; ./04-hello_heightmap.out`

This example introduces the loading and rendering of heightmaps. It also introduces
smooth camera movement and looking around with the mouse. The terrain is rendered without
color or texture and in wireframe. Adding color and texture is a good exercise for the beginning
reader.

[Code](src/examples/04-hello_heightmap)

![hello cube](img/04-hello_heightmap.tiff)

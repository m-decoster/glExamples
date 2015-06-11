# glExamples

This repository contains some OpenGL examples.
These examples have been written in C++ and use OpenGL 3.3.

Each example resides in its own folder. Code that is often reused, such as
code for compiling and linking shader programs, is put into a seperate folder and reused in the different examples.

This code has been written with OS X as a target. No special care has been
taken to make sure the code is cross-platform. While the examples might
not compile out of the box on your machine, the code can still be reused in
your projects. You should be able to just plug it into an OpenGL application
that compiles and runs on your machine.

## Dependencies

These examples have some dependencies:

* [GLFW 3.1.1](http://www.glfw.org)
* [SOIL](http://www.lonesock.net/soil.html)

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

1. [Hello Triangle](#01---Hello-Triangle)
2. [Hello Sprite](#02---Hello-Sprite)

### 01 - Hello Triangle

**Compile**: `make hello_triangle`  
**Run**: `cd bin; ./01-hello-triangle.out`

This example shows the minimal code needed to render a simple colored triangle.

[Code](src/examples/01-hello_triangle)

![hello triangle](img/01-hello_triangle.tiff)

### 02 - Hello Sprite

**Compile**: `make hello_sprite`  
**Run**: `cd bin; ./02-hello_sprite.out`

This example shows the minimal code needed to render a textured quad. It also uses
`glDrawElements` to reduce duplication of vertices.

[Code](src/examples/02-hello_sprite)

![hello sprite](img/02-hello_sprite.tiff)

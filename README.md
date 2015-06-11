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

## Compiling

### OS X

Make sure that you have compiled the GLFW dependency for your system as a
static library. You should now have a file called `libglfw3.a`. Place this
in a subdirectory `lib` of the root folder (on the same level as this `README.md` file).

Now you can use the command `make EXAMPLE_NAME` to compile an example. The
resulting binaries will be placed in the `build` folder.

## License

These examples are available under the MIT License. This is because public
domain is not recognized in every country. For more information, read
[this question and answer](http://programmers.stackexchange.com/questions/147111/what-is-wrong-with-the-unlicense).

## Examples

### Hello Triangle

**Compile**: `make hello_triangle`  
**Run**: `bin/01-hello-triangle.out`

This example shows the minimal code needed to render a simple colored triangle.

![hello triangle](img/01-hello_triangle.tiff)

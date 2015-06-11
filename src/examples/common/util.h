#ifndef UTIL_HEADER
#define UTIL_HEADER

#define GLFW_INCLUDE_GLCOREARB // Needed for OpenGL 3.3 on OS X
#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>

#include <cstdlib>
#include <iostream>

GLFWwindow* init(const char* exampleName, int width, int height);
GLuint loadImage(const char* fileName, int* w, int* h, int index);

#endif

#ifndef UTIL_HEADER
#define UTIL_HEADER

#define GLFW_INCLUDE_GLCOREARB // Needed for OpenGL 3.3 on OS X
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>

void error_callback(int error, const char* description);

#endif

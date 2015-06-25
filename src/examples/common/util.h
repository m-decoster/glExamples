#ifndef UTIL_HEADER
#define UTIL_HEADER

#define GLFW_INCLUDE_GLCOREARB // Needed for OpenGL 3.3 on OS X
#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>

#include <cstdlib>
#include <iostream>

class Camera;

GLFWwindow* init(const char* exampleName, int width, int height);
GLuint loadImage(const char* fileName, int* w, int* h, int index, bool alphaChannel);
GLuint loadCubeMap(const char* posX, const char* negX, const char* posY,
        const char* negY, const char* posZ, const char* negZ);
void setCamera(Camera* camera);
void updateCamera(int width, int height, GLFWwindow* window);

#endif

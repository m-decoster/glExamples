#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "util.h"

GLuint createShader(const char* src, GLenum shaderType);
GLuint createShaderProgram(GLuint vertex, GLuint fragment);
GLuint linkShader(GLuint program);
GLuint validateShader(GLuint program);

#endif

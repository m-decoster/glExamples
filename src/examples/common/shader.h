#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "util.h"

GLuint createShader(const char* src, GLenum shaderType);
GLuint createShaderProgram(GLuint vertex, GLuint fragment);
GLuint createShaderProgram(GLuint vertex, GLuint geometry, GLuint fragment);
bool linkShader(GLuint program);
bool validateShader(GLuint program);

#endif

#include "shader.h"

GLuint createShader(const char* src, GLenum shaderType)
{
    GLuint s = glCreateShader(shaderType);
    glShaderSource(s, 1, (const GLchar**)&src, NULL);
    glCompileShader(s);
    
    GLint status;
    glGetShaderiv(s, GL_COMPILE_STATUS, &status);
    if(!status)
    {
        std::cerr << "Compiling of shader failed: ";
        char log[512];
        glGetShaderInfoLog(s, 512, NULL, log);
        std::cerr << log << std::endl;
        return 0;
    }

    return s;
}

GLuint createShaderProgram(GLuint vertex, GLuint fragment)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    return program;
}

bool linkShader(GLuint program)
{
    glLinkProgram(program);
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(!status)
    {
        std::cerr << "Linking of shader failed: ";
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cerr << log << std::endl;
        return false;
    }
    return true;
}

bool validateShader(GLuint program)
{
    // OS X needs to have a vao bound before validating programs
    // I am not sure why
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glValidateProgram(program);
    int status;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if(!status)
    {
        std::cerr << "Shader validation failed: ";
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cerr << log << std::endl;

        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);

        return false;
    }

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);

    return true;
}

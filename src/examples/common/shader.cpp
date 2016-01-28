#include "shader.h"

GLuint createShader(const char* src, GLenum shaderType)
{
    // Create a shader and load the string as source code and compile it
    GLuint s = glCreateShader(shaderType);
    glShaderSource(s, 1, (const GLchar**)&src, NULL);
    glCompileShader(s);
    
    // Check compilation status: this will report syntax errors
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
    // Create a shader program and attach the vertex and fragment shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    return program;
}

GLuint createShaderProgram(GLuint vertex, GLuint geometry, GLuint fragment)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, geometry);
    glAttachShader(program, fragment);
    return program;
}

bool linkShader(GLuint program)
{
    // Link the program and check the status: this will report semantics errors
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

// Still here for backwards compatibility
bool validateShader(GLuint program)
{
    return true;
}

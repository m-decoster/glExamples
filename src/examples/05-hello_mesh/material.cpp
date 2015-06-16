#include "material.h"
#include <glm/gtc/type_ptr.hpp> 

Material::Material()
    : program(0)
{
}

Material::~Material()
{
    if(program)
    {
        glDeleteProgram(program);
    }
}

bool Material::load(const char* vertexSrc, const char* fragmentSrc)
{
    // We start by creating a vertex and fragment shader
    GLuint vertex = createShader(vertexSrc, GL_VERTEX_SHADER);
    if(!vertex)
    {
        return false;
    }
    GLuint fragment = createShader(fragmentSrc, GL_FRAGMENT_SHADER);
    if(!fragment)
    {
        return false;
    }
    // Now we must make a shader program: this program
    // contains both the vertex and the fragment shader
    GLuint program = createShaderProgram(vertex, fragment);
    if(!program)
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }
    // We link the program, just like your C compiler links
    // .o files
    bool result = linkShader(program);
    if(!result)
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(program);
        program = 0;
        return false;
    }
    // We make sure the shader is validated
    result = validateShader(program);
    if(!result)
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(program);
        program = 0;
        return false;
    }
    // Detach and delete the shaders, because we no longer need them
    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);

    return true;
}

bool Material::setUniform(const char* name, const glm::mat4& m)
{
    if(!program)
    {
        return false;
    }

    GLint loc = glGetUniformLocation(program, name);

    if(loc == -1)
    {
        return false;
    }

    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    return true;
}

bool Material::use()
{
    if(!program)
    {
        return false;
    }
    
    glUseProgram(program);
    return true;
}

void Material::stopUsing()
{
    glUseProgram(0);
}

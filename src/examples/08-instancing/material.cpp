#include "material.h"
#include "../common/shader.h"
#include <glm/gtc/type_ptr.hpp> 

Material::Material()
    : program(0), diffuse(0)
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
    program = createShaderProgram(vertex, fragment);
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
        std::cerr << "Program not set while trying to set uniform" << std::endl;
        return false;
    }

    GLint loc = glGetUniformLocation(program, name);

    if(loc == -1)
    {
        std::cerr << "Uniform '" << name << "' not found in shader" << std::endl;
        return false;
    }

    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    return true;
}

void Material::setDiffuseTexture(GLuint texture)
{
    diffuse = texture;
}

bool Material::use()
{
    if(!program)
    {
        std::cerr << "Tried to use material without program" << std::endl;
    }

    glUseProgram(program);
    return true;
}

bool Material::bind()
{
    if(!program || !diffuse)
    {
        std::cerr << "Set program and textures before binding a material!" << std::endl;
        return false;
    }

    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glUniform1i(glGetUniformLocation(program, "diffuse"), 0);
    return true;
}

void Material::stopUsing()
{
    glUseProgram(0);
}

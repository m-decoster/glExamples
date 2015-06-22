#include "skybox.h"
#include "../common/shader.h"
#include <glm/gtc/type_ptr.hpp>

static const char* VERTEX_SRC = "#version 330 core\n"
                                "layout(location=0) in vec3 position;"
                                "out vec3 fTexcoords;"
                                "uniform mat4 projection;"
                                "uniform mat4 view;"
                                "void main()"
                                "{"
                                "    gl_Position = projection * view * vec4(position, 1.0);"
                                "    gl_Position.z = 1.0;" // This allows us to render the skybox AFTER rendering everything else,
                                // which is a great optimization because OpenGL will have to evaluate less fragments
                                "    fTexcoords = position;" // Because we're working with a cubemap, the tex coordinates are the same as the position
                                "}";

static const char* FRAGMENT_SRC = "#version 330 core\n"
                                  "in vec3 fTexcoords;"
                                  "out vec4 outputColor;"
                                  "uniform samplerCube skybox;"
                                  "void main()"
                                  "{"
                                  "    outputColor = texture(skybox, fTexcoords);"
                                  "}";

Skybox::Skybox(GLuint cm)
    : cubemap(cm), vao(0)
{
    static float points[] = {
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

    glBindVertexArray(0);

    GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
    GLuint fragment = createShader(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
    program = createShaderProgram(vertex, fragment);
    linkShader(program);
    validateShader(program);
    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);
}

Skybox::~Skybox()
{
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
    }
    if(program)
    {
        glDeleteProgram(program);
    }
}

void Skybox::render(const glm::mat4& view, const glm::mat4& proj)
{
    glm::mat4 viewNew = glm::mat4(glm::mat3(view)); // Remove translation components

    glDepthFunc(GL_LEQUAL); // z = 1.0 in skybox, the default GL_LESS would discard the skybox

    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(viewNew));
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    // Standard cube rendering
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap); // GL_TEXTURE_CUBE_MAP instead of 2D!
    glUniform1i(glGetUniformLocation(program, "skybox"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // reset depth func
    glDepthFunc(GL_LESS);
}

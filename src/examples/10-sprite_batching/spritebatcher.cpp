#include "spritebatcher.h"
#include "../common/shader.h"
#include <cassert>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
                             
const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec2 position;"
                         "layout(location=1) in vec4 color;"
                         "layout(location=2) in vec2 texCoord;"
                         "layout(location=3) in mat4 model;"
                         "out vec2 fTexCoord;"
                         "out vec4 fColor;"
                         "uniform mat4 projection;"
                         "void main()"
                         "{"
                         "    gl_Position = projection * model * vec4(position, 0.0, 1.0);"
                         "    fColor = color;"
                         "    fTexCoord = texCoord;"
                         "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec2 fTexCoord;"
                           "in vec4 fColor;"
                           "uniform sampler2D tex;"
                           "out vec4 outputColor;"
                           "void main()"
                           "{"
                           "    outputColor = texture(tex, fTexCoord) * fColor;"
                           "}";

const int MAX_SPRITES = 5000;

#define VCTBO 0   // vertices, colour, texture coordinates buffer object
#define EBO   1     // element buffer object
#define MBO   2     // model buffer object (for the model matrix of each sprite)

SpriteBatcher::SpriteBatcher()
    : camera(NULL), vao(0), program(0), lastTexture(0), drawn(0)
{
    // Create shader program
    GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
    GLuint fragment = createShader(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
    program = createShaderProgram(vertex, fragment);
    linkShader(program);
    validateShader(program);

    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);

    // Create vertex array and prepare buffers for being updated >= once per frame
    glGenVertexArrays(1, &vao);
    glGenBuffers(3, buffers);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[VCTBO]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[EBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    
    // Attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VCTBO]);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2); // texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glEnableVertexAttribArray(3); // model matrix
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

SpriteBatcher::~SpriteBatcher()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(3, buffers);
    glDeleteProgram(program);
}

void SpriteBatcher::begin()
{
    glEnable(GL_BLEND);
}

void SpriteBatcher::end()
{
    render();

    glDisable(GL_BLEND);
}

void SpriteBatcher::draw(Sprite* sprite)
{
    if(drawn > MAX_SPRITES || lastTexture != sprite->getTexture())
    {
        render();
    }

    // We first set up the vertex data as interleaved
    // positions, colors and texture coordinates.
    // We also set up the element buffer data.
    unsigned char r, g, b, a;
    sprite->getColor(&r, &g, &b, &a);
    float rf = r / 255.f, gf = g / 255.f, bf = b / 255.f, af = a / 255.f;
    int x, y, w, h, tW, tH;
    sprite->getTextureRectangle(&x, &y, &w, &h);
    sprite->getTextureDimensions(&tW, &tH);
    float _vertices[] =
    {
        // x  y     r   g   b   a   u                      v
        0.0f, 0.0f, rf, gf, bf, af,  x / (float)tW,        y / (float)tH,
        1.0f, 0.0f, rf, gf, bf, af, (x + w) / (float)tW,   y / (float)tH,
        0.0f, 1.0f, rf, gf, bf, af,  x / (float)tW,       (y + h) / (float)tH,
        1.0f, 1.0f, rf, gf, bf, af, (x + w) / (float)tW,  (y + h) / (float)tH
    };

    static const GLuint _indices[] = // Always the same
    {
        0, 1, 2,
        2, 1, 3
    };

    for(int v = 0; v < 32; ++v)
    {
        vertices.push_back(_vertices[v]);
    }

    for(int e = 0; e < 6; ++e)
    {
        // Note the sum: 4 * drawn indicates the number of vertices that we have
        // already added to the vertices vector. If we do not add this sum,
        // we would just draw the first sprite over and over again
        indices.push_back(_indices[e] + 4 * drawn);
    }

    // Now add the model matrix for each vertex
    // Other possibilities include:
    // 1. Transform the vertices on the CPU
    // 2. Send 'offset, scale, angle' vertices and create the model matrix in the vertex shader
    glm::mat4 model = sprite->getModelMatrix();
    modelMatrices.push_back(model);
    modelMatrices.push_back(model);
    modelMatrices.push_back(model);
    modelMatrices.push_back(model);

    ++drawn;
    lastTexture = sprite->getTexture();
}

void SpriteBatcher::setCamera(Camera* c)
{
    camera = c;
}

void SpriteBatcher::render()
{
    if(vertices.empty())
    {
        return;
    }

    glBindVertexArray(vao);
    glUseProgram(program);

    // Send the vertices
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VCTBO]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);

    // Send the indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[EBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_DYNAMIC_DRAW);

    // Send the model matrices
    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Set the projection uniform
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(camera->getProjection()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    // Draw
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    drawn = 0;
}

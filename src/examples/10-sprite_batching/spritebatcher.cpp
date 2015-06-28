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
                         "    gl_Position = model * projection * vec4(position, 0.0, 1.0);"
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

#define VBO 0   // vertex buffer object
#define EBO 1   // element buffer object
#define MBO 2   // model buffer object (for the model matrix of each sprite)
#define CBO 3   // color buffer object
#define TBO 4   // texture coordinate buffer object

SpriteBatcher::SpriteBatcher()
    : camera(NULL), vao(0), program(0)
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
    glGenBuffers(5, buffers);

    glBindVertexArray(vao);

    float vertices[] =
    {
        -0.5f, -0.5f,
         0.5f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f
    };

    GLuint indices[] =
    {
        0, 1, 2,
        2, 1, 3
    };

    // Vertices are the same for each sprite
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Indices are the same for each sprite
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[EBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // Color vectors differ for each sprite
    glBindBuffer(GL_ARRAY_BUFFER, buffers[CBO]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    // Texture coordinates differ for each sprite
    glBindBuffer(GL_ARRAY_BUFFER, buffers[TBO]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    // Model matrices differ for each sprite
    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    
    // Attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO]);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[CBO]);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[TBO]);
    glEnableVertexAttribArray(2); // texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glEnableVertexAttribArray(3); // model matrix
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

SpriteBatcher::~SpriteBatcher()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(5, buffers);
    glDeleteProgram(program);
}

void SpriteBatcher::begin()
{
    // Do nothing
}

void SpriteBatcher::end()
{
    render();
    queue.clear();
}

void SpriteBatcher::draw(Sprite* sprite)
{
    queue.push_back(sprite);
    if(queue.size() >= MAX_SPRITES)
    {
        render();
        queue.clear();
    }
}

void SpriteBatcher::setCamera(Camera* c)
{
    camera = c;
}

void SpriteBatcher::render()
{
    // Sort the queue: first by z, then by texture
    std::sort(queue.begin(), queue.end(),
            [](const Sprite* a, const Sprite* b) -> bool
            {
                return a->compare(*b);
            });

    /*
     * In this implementation I have chosen to send the model data for each vertex.
     * In another implementation I transformed the vertices on the CPU.
     * Both have their advantages and drawbacks. I chose this method because
     * it's easy and doesn't show any real performance issues
     */
    std::vector<glm::mat4> models;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> texCoords;

    for(auto it = queue.begin(); it != queue.end(); ++it)
    {
        // We have to add the model matrix 4 times, once for each vertex
        models.push_back((*it)->getModelMatrix());
        models.push_back((*it)->getModelMatrix());
        models.push_back((*it)->getModelMatrix());
        models.push_back((*it)->getModelMatrix());
        
        // Now for the colors
        glm::vec4 color;
        unsigned char r, g, b, a;

        (*it)->getColor(&r, &g, &b, &a);

        color.r = r / 255.0f;
        color.g = g / 255.0f;
        color.b = b / 255.0f;
        color.a = a / 255.0f;

        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);

        // Now for the texture coordinates
        int x, y, w, h;
        (*it)->getTextureRectangle(&x, &y, &w, &h);
        int tW, tH;
        (*it)->getTextureDimensions(&tW, &tH);

        texCoords.emplace_back(x / (float)tW, y / (float)tH);
        texCoords.emplace_back((x + w) / (float)tW, y / (float)tH);
        texCoords.emplace_back(x / (float)tW, (y + h) / (float)tH);
        texCoords.emplace_back((x + w) / (float)tW, (y + h) / (float)tH);
    }

    glBindVertexArray(vao);
    glUseProgram(program);

    // Send the color data
    glBindBuffer(GL_ARRAY_BUFFER, buffers[CBO]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), &colors[0], GL_DYNAMIC_DRAW);

    // Send the texture coordinate data
    glBindBuffer(GL_ARRAY_BUFFER, buffers[TBO]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), &texCoords[0], GL_DYNAMIC_DRAW);

    // Send the model matrices
    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), &models[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Set the projection uniform (there is no need for a view matrix in 2D)
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(camera->getProjection()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, (*queue.begin())->getTexture());
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    // Draw
    glDrawElements(GL_TRIANGLES, queue.size() * 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

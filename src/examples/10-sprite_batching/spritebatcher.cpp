#include "spritebatcher.h"
#include <cassert>
#include <algorithm>

const char* VERTEX_SRC = "";
const char* FRAGMENT_SRC = "";

const int MAX_SPRITES = 5000;

#define VBO 0   // vertex buffer object
#define EBO 1   // element buffer object
#define MBO 2   // model buffer object (for the model matrix of each sprite)

SpriteBatcher::SpriteBatcher()
    : camera(NULL), vao(0), vbo(0), ebo(0), mbo(0)
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

    glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[EBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STREAM_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[VBO]);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (void*)(7 * sizeof(float)), (void*)0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (void*)(7 * sizeof(float)), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); // texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (void*)(7 * sizeof(float)), (void*)(5 * sizeof(float)));

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
}

SpriteBatcher::~SpriteBatcher()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(3, buffers);
}

void SpriteBatcher::begin()
{
}

void SpriteBatcher::end()
{
    render();
}

void SpriteBatcher::draw(Sprite& sprite)
{
    queue.push_back(sprite);
}

void SpriteBatcher::render()
{
    // Sort the queue: first by z, then by texture
    std::sort(queue.begin(), queue.end(),
            [](const Sprite& a, const Sprite& b) -> bool
            {
                return a.compare(b);
            });
    
}

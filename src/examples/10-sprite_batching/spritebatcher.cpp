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
    // Model matrices differ for each sprite
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
    // Do nothing
}

void SpriteBatcher::end()
{
    render();
    queue.clear();
}

void SpriteBatcher::draw(Sprite& sprite)
{
    queue.push_back(sprite);
}

void SpriteBatcher::setCamera(Camera* c)
{
    camera = c;
}

void SpriteBatcher::render()
{
    // Sort the queue: first by z, then by texture
    std::sort(queue.begin(), queue.end(),
            [](const Sprite& a, const Sprite& b) -> bool
            {
                return a.compare(b);
            });

    /*
     * In this implementation I have chosen to send the model data for each vertex.
     * In another implementation I transformed the vertices on the CPU.
     * Both have their advantages and drawbacks. I chose this method because
     * it's easy and doesn't show any real performance issues
     */
    std::vector<glm::mat4> models;

    for(auto it = queue.begin(); it != queue.end(); ++it)
    {
        // We have to add the model matrix 4 times, once for each vertex
        models.push_back(*it);
        models.push_back(*it);
        models.push_back(*it);
        models.push_back(*it);
    }

    glBindVertexArray(vao);

    // Send the model matrices
    glBindBuffer(GL_ARRAY_BUFFER, buffers[MBO]);
    glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), &models[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Set the view and projection uniforms
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(camera->getView()));
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(camera->getProjection()));

    // Draw
    glDrawElements(GL_TRIANGLES, queue.size() * 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

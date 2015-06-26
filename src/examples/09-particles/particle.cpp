#include "particle.h"
#include "../common/shader.h"

const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec2 position;"
                         "layout(location=1) in vec3 transform;" // x, y, size
                         "void main()"
                         "{"
                         "    gl_Position = vec4(position + transform.xy, 0.0, 1.0);"
                         "    gl_Position.x *= transform.z;"
                         "    gl_Position.y *= transform.z;"
                         "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "out vec4 outColor;"
                           "void main()"
                           "{"
                           "    outColor = vec4(1.0f);"
                           "}";

ParticleEmitter::ParticleEmitter(int max, float ivl, const glm::vec2& pos, float pLife)
    : vao(0), program(0), lastIndex(0), interval(ivl), lastEmission(0.0f), particleLife(pLife), position(pos), emitting(false)
{
    particles.resize(max);
    particleBuffer.resize(max * 3); // x, y, size (layout(location=1) in vec3 transform)

    // Shader
    GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
    GLuint fragment = createShader(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
    program = createShaderProgram(vertex, fragment);
    linkShader(program);
    validateShader(program);

    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);

    // VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffers[2]; // vbo, ebo
    glGenBuffers(2, buffers);

    glGenBuffers(1, &transformBuffer);

    float vertices[] =
    {
        -0.5f, 0.5f,
        0.5f, 0.5f,
        0.5f, -0.5f,
        -0.5f, -0.5f
    };

    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Bind the third buffer. We will use this buffer to set positions of the
    // active particles each frame. Because we update the buffer each frame, we use
    // GL_STREAM_DRAW, which lets OpenGL optimize the buffer
    glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
    glDeleteBuffers(2, buffers);
}

ParticleEmitter::~ParticleEmitter()
{
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
    }
    if(transformBuffer)
    {
        glDeleteBuffers(1, &transformBuffer);
    }
    if(program)
    {
        glDeleteProgram(program);
    }
}

void ParticleEmitter::start()
{
    emitting = true;
}

void ParticleEmitter::stop()
{
    emitting = false;
}

void ParticleEmitter::update(float deltaTime)
{
    if(emitting)
    {
        lastEmission += deltaTime;
        if(lastEmission >= interval)
        {
            emit();
            lastEmission -= interval;
        }
    }

    for(int i = 0; i < particles.size(); ++i)
    {
        particles.at(i).life -= deltaTime;
        // Gravity and a random horizontal offset
        particles.at(i).speed += glm::vec2((rand() % 100) / 25.0f - 2.0f, -0.981f) * deltaTime;
        particles.at(i).position += particles.at(i).speed * deltaTime;
    }
}

void ParticleEmitter::render()
{
    glUseProgram(program);
    glBindVertexArray(vao);

    int liveCount = 0;

    for(int i = 0; i < particles.size(); ++i)
    {
        // Only render live particles
        if(particles.at(i).life > 0.0f)
        {
            particleBuffer[3 * liveCount] = particles.at(i).position.x;
            particleBuffer[3 * liveCount + 1] = particles.at(i).position.y;
            particleBuffer[3 * liveCount + 2] = 0.05f; // Right now, all particles have the same size
            // You can easily change the size of the particles based on life, distance from origin, etc.
            ++liveCount;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
    // Remap storage (optimization. Google "buffer orphaning opengl" for more information)
    glBufferData(GL_ARRAY_BUFFER, particles.size() * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
    // Update the position data. Note that we only pass liveCount * 3 * sizeof(float)
    // elements to the buffer. The first liveCount * 3 * sizeof(float) are the only live
    // particles' data
    glBufferSubData(GL_ARRAY_BUFFER, 0, liveCount * 3 * sizeof(float), &particleBuffer[0]);

    // Do instanced rendering. Only render liveCount particles
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, liveCount);

    glBindVertexArray(0);
    glUseProgram(0);
}

void ParticleEmitter::emit()
{
    // Find the first particle that we can replace
    int i;
    for(i = lastIndex; i < particles.size(); ++i)
    {
        // If the particle is almost dead
        if(particles.at(i).life < 0.001f)
        {
            lastIndex = i;
            break;
        }
    }

    if(i == particles.size() - 1)
    {
        // We passed the end of the vector without finding a suitable
        // particle. The particle at 0 is in this case the oldest.
        lastIndex = 0;
    }

    particles.at(lastIndex).position = position;
    particles.at(lastIndex).life = particleLife;
}

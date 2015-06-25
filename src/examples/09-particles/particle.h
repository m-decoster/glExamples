#ifndef PARTICLE_HEADER
#define PARTICLE_HEADER

#include "../common/util.h"
#include <glm/glm.hpp>
#include <vector>

struct Particle
{
    Particle() : life(0.0f) {};

    glm::vec2 position, speed;
    glm::vec4 color;
    float life; // When this goes below zero, the particle is considered dead
};

class ParticleEmitter
{
public:
    /**
     * Create a new ParticleEmitter
     * maxCount: max particles
     * interval: interval between emissions
     * position: where new particles are emitted
     * particleLife: time in milliseconds before particles die
     */
    ParticleEmitter(int maxCount, float interval, const glm::vec2& position, float particleLife);
    ~ParticleEmitter();

    void start();
    void stop();
    void update(float deltaTime);
    void render();
private:
    std::vector<Particle> particles;
    std::vector<float> particleBuffer;
    GLuint vao, program, transformBuffer;
    int lastIndex; // Index of the last accessed particle. We use this to find a spot
    // in the particles vector to create a new particle
    float interval, lastEmission, particleLife;
    glm::vec2 position;
    bool emitting;

    void emit();
};

#endif

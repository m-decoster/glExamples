#ifndef MESH_HEADER
#define MESH_HEADER

#include "../common/util.h"
#include <glm/glm.hpp>

class Mesh
{
public:
    Mesh();
    ~Mesh();

    bool load(const char* fileName);

    void setPosition(float x, float y, float z);
    void setScale(float x, float y, float z);
    void setAngle(float x, float y, float z);

    void render();

    glm::mat4 getModelMatrix();
private:
    glm::vec3 position, scale, angle;
    GLuint vao;
};

#endif

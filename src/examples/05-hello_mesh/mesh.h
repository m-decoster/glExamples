#ifndef MESH_HEADER
#define MESH_HEADER

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

    const glm::mat4& getModelMatrix() const;
private:
    glm::mat4 modelMatrix;
    GLuint vao;
};

#endif

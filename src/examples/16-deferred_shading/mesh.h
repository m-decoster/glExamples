#ifndef MESH_HEADER
#define MESH_HEADER

#include "../common/util.h"
#include <glm/glm.hpp>
#include <vector>

class Mesh
{
public:
    Mesh();
    ~Mesh();

    bool load(const char* fileName);
    void setInstances(int numMeshes, std::vector<glm::mat4> models);

    void render();
private:
    int numIndices, numMeshes;
    GLuint vao;
};

#endif

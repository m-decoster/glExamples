#ifndef MESH_HEADER
#define MESH_HEADER

#include "../common/util.h"
#include <glm/glm.hpp>
#include <vector>

/*
 * The changes to this class include:
 * the render code will now use glDrawElementsInstanced;
 * there is a setInstances method that allows us to set the different model matrices;
 * we no longer need getModelMatrix(), so it has been removed;
 * there is a new field numMeshes that indicates the number of instances;
 * we no longer store the individual positions for this example, so that has been removed too
 */

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

#ifndef SCENE_HEADER
#define SCENE_HEADER

#include "../common/util.h"
#include <glm/glm.hpp>
#include <vector>

class Mesh;
class Material;

class Scene
{
public:
    Scene();
    ~Scene();

    bool load(const char* fileName);

    void render(Material* m);
private:
    std::vector<Mesh*> m_meshes;
};

#endif

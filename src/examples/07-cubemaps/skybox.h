#ifndef SKYBOX_HEADER
#define SKYBOX_HEADER

#include "../common/util.h"
#include <glm/glm.hpp>

class Skybox
{
public:
    Skybox(GLuint cubemap);
    ~Skybox();

    void render(const glm::mat4& view, const glm::mat4& projection);
private:
    GLuint cubemap;
    GLuint vao;
    GLint program;
};

#endif

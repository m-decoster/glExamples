#ifndef MATERIAL_HEADER
#define MATERIAL_HEADER

#include "../common/util.h"
#include <glm/glm.hpp>

class Material
{
public:
    Material();
    ~Material();

    bool load(const char* vertexSrc, const char* fragmentSrc);

    bool setUniform(const char* name, const glm::mat4& m);
    bool use();
    bool bind();
    void stopUsing();

    void setDiffuseTexture(GLuint texture);
private:
    GLuint program;
    GLuint diffuse;
};

#endif

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

    /**
     * Set a uniform to a certain value
     */
    bool setUniform(const char* name, const glm::mat4& m);
    bool setUniform(const char* name, const glm::vec3& m);
    /**
     * Use the underlying shader program
     */
    bool use();
    /**
     * Use the underlying shader program and bind
     * the textures
     */
    bool bind();
    void stopUsing();

    void setDiffuseTexture(GLuint texture);

    GLuint program;
    GLuint diffuse;
};

#endif

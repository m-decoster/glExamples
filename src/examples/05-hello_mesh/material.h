#ifndef MATERIAL_HEADER
#define MATERIAL_HEADER

#include <glm/glm.hpp>

class Material
{
public:
    Material();
    ~Material();

    bool load(const char* vertexSrc, const char* fragmentSrc);

    bool setUniform(const char* name, const glm::mat4& m);
    bool use();
    void stopUsing();
private:
    GLuint program;
};

#endif

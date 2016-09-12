#ifndef PS_UTIL_H
#define PS_UTIL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "../common/util.h"

struct PointLight
{
    PointLight(const glm::vec3& pos, const glm::vec3& col, const glm::vec3& att)
        : position(pos), color(col), attenuation(att) {};
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 attenuation;
};

#endif

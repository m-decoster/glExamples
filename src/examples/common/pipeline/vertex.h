#ifndef VERTEX_HEADER
#define VERTEX_HEADER

#include <glm/glm.hpp>

struct Vertex
{
    Vertex(const glm::vec4& position, const glm::vec4& color, const glm::vec2& texCoord)
        : position(position), color(color), texCoord(texCoord) {};
    
    glm::vec4 position, color;
    glm::vec2 texCoord;
};

#endif

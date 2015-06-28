#ifndef SPRITE_HEADER
#define SPRITE_HEADER

#include <glm/glm.hpp>
#include "../common/util.h"

class Sprite
{
public:
    Sprite(GLuint texture);
    ~Sprite();

    void setPosition(float x, float y, float z);
    void setScale(float x, float y);
    void setAngle(float angle);
    void setTextureRectangle(int x, int y, int w, int h);
    void setColor(char r, char g, char b, char a);

    GLuint getTexture() const;
    const glm::mat4& getModelMatrix();
    void getTextureRectangle(int* x, int* y, int* w, int* h) const;

    bool compare(const Sprite& other) const;
private:
    bool dirty;
    GLuint texture;
    glm::vec2 scale;
    float angle;
    glm::vec3 position;
    char r, g, b, a;
    glm::mat4 model;
    int x, y, w, h; // Texture rectangle
};

#endif

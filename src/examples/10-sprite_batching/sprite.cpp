#include "sprite.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Sprite::Sprite(GLuint texture, int tW, int tH)
    : dirty(true), texture(texture), scale(1.0f, 1.0f), angle(0.0f), position(0.0f, 0.0f, 0.0f),
    r(255), g(255), b(255), a(255), textureWidth(tW), textureHeight(tH)
{
}

Sprite::~Sprite()
{
}

void Sprite::setPosition(float x, float y, float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
    dirty = true;
}

void Sprite::setScale(float x, float y)
{
    scale.x = x;
    scale.y = y;
    dirty = true;
}

void Sprite::setAngle(float a)
{
    angle = a;
    dirty = true;
}

void Sprite::setTextureRectangle(int x_, int y_, int w_, int h_)
{
    x = x_;
    y = y_;
    w = w_;
    h = h_;
}

void Sprite::setColor(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_)
{
    r = r_;
    g = g_;
    b = b_;
    a = a_;
}

GLuint Sprite::getTexture() const
{
    return texture;
}

const glm::mat4& Sprite::getModelMatrix()
{
    if(dirty)
    {
        model = glm::rotate(glm::mat4(), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(scale.x, scale.y, 1.0f));
        dirty = false;
    }
    return model;
}

void Sprite::getTextureRectangle(int* x_, int* y_, int* w_, int* h_) const
{
    *x_ = x;
    *y_ = y;
    *w_ = w;
    *h_ = h;
}

bool Sprite::compare(const Sprite& other) const
{
    if(std::abs(position.z - other.position.z) < 0.0001f)
    {
        return position.z < other.position.z;
    }

    return texture < other.texture;
}

void Sprite::getColor(unsigned char* r_, unsigned char* g_, unsigned char* b_, unsigned char* a_) const
{
    *r_ = r;
    *g_ = g;
    *b_ = b;
    *a_ = a;
}

void Sprite::getTextureDimensions(int* tW, int* tH)
{
    *tW = textureWidth;
    *tH = textureHeight;
}

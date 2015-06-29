#ifndef SPRITEBATCHER_HEADER
#define SPRITEBATCHER_HEADER

#include "sprite.h"
#include "../common/camera.h"
#include "../common/util.h"
#include <vector>

class SpriteBatcher
{
public:
    SpriteBatcher();
    ~SpriteBatcher();

    void begin();
    void end();
    void draw(Sprite* sprite);

    void setCamera(Camera* camera);
private:
    Camera* camera;
    GLuint vao, program;
    GLuint buffers[3];
    std::vector<float> vertices;
    std::vector<GLuint> indices;
    GLuint lastTexture;
    int drawn;

    void render();
};

#endif

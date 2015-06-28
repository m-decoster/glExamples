#ifndef SPRITEBATCHER_HEADER
#define SPRITEBATCHER_HEADER

#include "sprite.h"
#include "../common/camera.h"
#include "../common/util.h"

class SpriteBatcher
{
public:
    SpriteBatcher();
    ~SpriteBatcher();

    void begin();
    void end();
    void draw(Sprite& sprite);

    void setCamera(Camera* camera);
private:
    Camera* camera;
    GLuint vao;
    GLuint buffers[3];
    std::vector<Sprite&> queue;

    void render();
};

#endif

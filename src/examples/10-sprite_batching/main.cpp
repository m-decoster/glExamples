#include "spritebatcher.h"
#include "sprite.h"
#include "../common/util.h"
#include "../common/camera.h"

#define SQRT_NUM_SPRITES 40

int main(void)
{
    GLFWwindow* window;

    window = init("Sprite batching", 640, 480);
    if(!window)
    {
        return -1;
    }

    Camera camera(CAMERA_ORTHOGONAL, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    setCamera(&camera);

    int w, h;
    GLuint texture = loadImage("spritesheet.png", &w, &h, 0, true);
    if(!texture)
    {
        std::cerr << "Could not load texture" << std::endl;
        return -1;
    }

    SpriteBatcher spritebatch;

    std::vector<Sprite> sprites;
    for(int i = 0; i < SQRT_NUM_SPRITES; ++i)
    {
        for(int j = 0; j < SQRT_NUM_SPRITES; ++j)
        {
            Sprite s(texture, w, h);
            s.setPosition(i, j, rand() % 10 / 10.0f);
            s.setColor(rand() % 10 / 10.0f, rand() % 10 / 10.0f, rand() % 10 / 10.0f, 1.0f);
            s.setTextureRectangle(rand() % (w - 10), rand() % (h - 10), 10, 10);
            sprites.push_back(s);
        }
    }

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        updateCamera(640, 480, window);

        glClear(GL_COLOR_BUFFER_BIT);

        spritebatch.begin();
        
        for(auto it = sprites.begin(); it != sprites.end(); ++it)
        {
            spritebatch.draw(*it);
        }

        spritebatch.end();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

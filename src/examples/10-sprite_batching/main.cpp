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

    Camera camera(CAMERA_ORTHOGONAL, 45.0f, -1.0f, 10000.0f, 640.0f, 480.0f);

    int w, h;
    GLuint texture = loadImage("spritesheet.png", &w, &h, 0, true);
    if(!texture)
    {
        std::cerr << "Could not load texture" << std::endl;
        return -1;
    }

    SpriteBatcher spritebatch;
    spritebatch.setCamera(&camera);

    std::vector<Sprite*> sprites;
    for(int i = 0; i < SQRT_NUM_SPRITES; ++i)
    {
        for(int j = 0; j < SQRT_NUM_SPRITES; ++j)
        {
            Sprite* s = new Sprite(texture, w, h);
            s->setPosition(rand() % 640, rand() % 320, 0.0f);
            s->setScale(50.0f, 50.0f);
            s->setAngle((float)(rand() % 360));
            s->setTextureRectangle(0, 0, w, h);
            sprites.push_back(s);
        }
    }

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        spritebatch.begin();
        
        for(auto it = sprites.begin(); it != sprites.end(); ++it)
        {
            spritebatch.draw(*it);
        }

        spritebatch.end();

        glfwSwapBuffers(window);
        glfwPollEvents();

        std::cout << glfwGetTime() << std::endl;
        glfwSetTime(0.0);
    }

    // Cleanup
    for(auto it = sprites.begin(); it != sprites.end(); ++it)
    {
        delete *it;
    }
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

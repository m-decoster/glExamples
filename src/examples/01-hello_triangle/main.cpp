#include "../common/util.h"

int main(void)
{
    GLFWwindow* window;

    window = init("Hello Triangle", 640, 480);
    if(!window)
    {
        return -1;
    }

    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

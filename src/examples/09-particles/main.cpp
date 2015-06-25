#include "../common/util.h"
#include "particle.h"

int main(void)
{
    GLFWwindow* window;

    window = init("Particles", 640, 480);
    if(!window)
    {
        return -1;
    }

    // Maximum of 1000 particles, spawn every tenth of a second at (0,0), last for 3 seconds
    ParticleEmitter emitter(1000, 100.0f, glm::vec2(0.0f, 0.0f), 3000.0f);

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    bool emitting = false;

    while(!glfwWindowShouldClose(window))
    {
        // Press 's' to start or stop emitting
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            if(emitting)
            {
                emitter.stop();
            }
            else
            {
                emitter.start();
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        // particle.cpp
        emitter.update((float)glfwGetTime());
        emitter.render();

        glfwSetTime(0.0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

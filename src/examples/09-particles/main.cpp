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
    ParticleEmitter emitter(1000, 0.1f, glm::vec2(0.0f, 0.0f), 3.0f);

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    emitter.start();

    while(!glfwWindowShouldClose(window))
    {
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

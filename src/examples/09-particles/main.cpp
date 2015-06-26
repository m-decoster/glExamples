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

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Maximum of 10000 particles, spawn every one hundredth of a second at (0,0), last for 5 seconds
    ParticleEmitter emitter(10000, 0.01f, glm::vec2(0.0f, 0.0f), 5.0f);

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

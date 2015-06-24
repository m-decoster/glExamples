#include "../common/util.h"
#include "../common/shader.h"
#include "skybox.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int main(void)
{
    GLFWwindow* window;

    window = init("Cubemaps", 640, 480);
    if(!window)
    {
        return -1;
    }

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)640/(float)480, 0.1f, 1000.0f);
    glm::mat4 view;

    // We will need to enable depth testing, so that OpenGL draws further
    // vertices first
    glEnable(GL_DEPTH_TEST);

    GLuint cubemap = loadCubeMap("cm_xp.png", "cm_xn.png", "cm_yp.png", "cm_yn.png", "cm_zp.png", "cm_zn.png");
    if(!cubemap)
    {
        std::cerr << "Could not load cubemap" << std::endl;
        return -1;
    }
    Skybox skybox(cubemap);

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate the view to show the 3D skybox effect
        view = glm::rotate(view, glm::radians(15.0f) * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        glfwSetTime(0.0);

        skybox.render(view, proj);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

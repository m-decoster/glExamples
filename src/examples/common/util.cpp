#include "util.h"

static void error_callback(int error, const char* description)
{
    std::cerr << description << std::endl;
}

GLFWwindow* init(const char* exampleName, int width, int height)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
    {
        return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // OS X
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, exampleName, NULL, NULL);
    if(!window)
    {
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    std::cout << "Using OpenGL version " << glGetString(GL_VERSION) << std::endl;

    return window;
}

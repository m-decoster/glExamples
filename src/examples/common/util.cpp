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

    glEnable(GL_TEXTURE_2D);

    return window;
}

GLuint loadImage(const char* fileName, int* w, int* h, int index)
{
    GLuint tex;
    unsigned char* img = SOIL_load_image(fileName, w, h, NULL, SOIL_LOAD_RGB);
    if(!img)
    {
        std::cerr << "Error loading image " << fileName << ": " << SOIL_last_result() << std::endl;
        return 0;
    }

    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *w, *h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
    SOIL_free_image_data(img);

    return tex;
}

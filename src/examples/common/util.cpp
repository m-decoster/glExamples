#include "util.h"
#include "camera.h"

static const float SPEED = 50.0f;
static const float MOUSE_SPEED = 0.025f;

static Camera* camera;

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

GLuint loadCubeMap(const char* posX, const char* negX, const char* posY,
        const char* negY, const char* posZ, const char* negZ)
{
    return SOIL_load_OGL_cubemap(posX, negX, posY, negY, posZ, negZ,
            SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
}

void setCamera(Camera* cam)
{
    camera = cam;
}

void updateCamera(int width, int height, GLFWwindow* window)
{
    float deltaTime = (float)glfwGetTime();
    glfwSetTime(0.0);

    // Get mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwSetCursorPos(window, width / 2, height / 2);
    float horizontalAngle = camera->getHorizontalAngle();
    float verticalAngle = camera->getVerticalAngle();
    horizontalAngle += MOUSE_SPEED * deltaTime * (float)(width / 2 - xpos);
    verticalAngle   += MOUSE_SPEED * deltaTime * (float)(height / 2 - ypos);

    camera->setHorizontalAngle(horizontalAngle);
    camera->setVerticalAngle(verticalAngle);

    // Get key input
    glm::vec3 direction = camera->getDirectionVector();
    glm::vec3 position = camera->getPosition();
    glm::vec3 right = camera->getRightVector();
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        position += direction * deltaTime * SPEED;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        position -= direction * deltaTime * SPEED;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        position -= right * deltaTime * SPEED;
    }
    else if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        position += right * deltaTime * SPEED;
    }
    camera->setPosition(position.x, position.y, position.z);
}

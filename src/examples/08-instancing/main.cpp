#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include "material.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUM_ASTEROIDS 2000
#define SEED 1993

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec3 position;"          // Vertex position (x, y, z)
                          "layout(location=1) in vec2 texcoord;"          // Texture coordinate (u, v)
                          "layout(location=2) in mat4 model_inst;"       // Model matrix for this instance
                          "uniform mat4 view;"
                          "uniform mat4 projection;"
                          "out vec2 fTexcoord;"                           // Pass to fragment shader
                          "void main()"
                          "{"
                          "    fTexcoord = texcoord;"                     // Pass texcoord to fragment shader
                          "    gl_Position = projection * view * model_inst * vec4(position, 1.0);"     // Place vertex at (x, y, z, 1) and then transform it according to the projection, view and model matrices
                          "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec2 fTexcoord;"                           // From the vertex shader
                           "uniform sampler2D diffuse;"                       // The texture
                           "out vec4 outputColor;"                        // The color of the resulting fragment
                           "void main()"
                           "{"
                           "    outputColor = texture(diffuse, fTexcoord);"
                           "}";

int main(void)
{
    GLFWwindow* window;

    window = init("Instancing", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Hide the cursor (escape will exit the application)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the camera
    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    setCamera(&camera);

    // Load the material
    Material mat;
    if(!mat.load(VERTEX_SRC, FRAGMENT_SRC))
    {
        std::cerr << "Could not load shaders" << std::endl;
        return -1;
    }
    mat.use();
    int w, h;
    GLuint texture = loadImage("asteroid.png", &w, &h, 0, false);
    if(!texture)
    {
        std::cerr << "Could not load texture" << std::endl;
        return -1;
    }
    mat.setDiffuseTexture(texture);

    // Load the mesh
    Mesh mesh;
    if(!mesh.load("asteroid.obj"))
    {
        std::cerr << "Could not load mesh" << std::endl;
    }

    // Set random positions for the asteroids
    std::vector<glm::mat4> models;
    models.resize(NUM_ASTEROIDS);
    srand(SEED);
    for(int i = 0; i < NUM_ASTEROIDS; ++i)
    {
        glm::mat4 model;

        // Translate
        model = glm::translate(model, glm::vec3(rand() % 100 - 50.0f, rand() % 100 - 50.0f, rand() % 100));

        // Scale
        float scale = (rand() % 200) / 100.0f + 0.1f;
        model = glm::scale(model, glm::vec3(scale, scale, scale));

        // Rotate
        model = glm::rotate(model, glm::radians((float)(rand() % 100)), glm::vec3(1.0f, 1.0f, 1.0f));

        models[i] = model;
    }

    mesh.setInstances(NUM_ASTEROIDS, models);
    
    // Set the clear color to a light grey
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        updateCamera(640, 480, window);

        // Clear (note the addition of GL_DEPTH_BUFFER_BIT)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Upload the MVP matrices
        mat.bind();
        mat.setUniform("view", camera.getView());
        mat.setUniform("projection", camera.getProjection());

        mesh.render();

        // Swap buffers to show current image on screen (for more information google 'backbuffer')
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

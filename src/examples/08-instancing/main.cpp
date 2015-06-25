#include "../common/util.h"
#include "../common/shader.h"
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

    // We will need to enable depth testing, so that OpenGL draws further
    // vertices first
    glEnable(GL_DEPTH_TEST);

    // Create a perspective projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)640/(float)480, 0.1f, 1000.0f);
    // vertex_clip = M_projection . M_view . M_model . vertex_local
    // Create the view matrix
    glm::mat4 view;
    // Move the scene so that we can see the mesh
    view = glm::lookAt(glm::vec3(0.0f, 50.0f, 200.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // In this example, the shader and vertex array object are set up in another class
    // see mesh.h & material.h

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
        // Clear (note the addition of GL_DEPTH_BUFFER_BIT)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSetTime(0.0);

        // Upload the MVP matrices
        mat.bind();
        mat.setUniform("view", view);
        mat.setUniform("projection", proj);

        mesh.render();

        // Tip: if nothing is drawn, check the return value of glGetError and google it

        // Swap buffers to show current image on screen (for more information google 'backbuffer')
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

#include "../common/util.h"
#include "../common/shader.h"
#include "material.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec3 position;"          // Vertex position (x, y, z)
                          "layout(location=1) in vec3 color;"             // Vertex color (r, g, b)
                          "layout(location=2) in vec2 texcoord;"          // Texture coordinate (u, v)
                          "uniform mat4 model;"
                          "uniform mat4 view;"
                          "uniform mat4 projection;"
                          "out vec3 fColor;"                              // Vertex shader has to pass color to fragment shader
                          "out vec2 fTexcoord;"                           // Pass to fragment shader
                          "void main()"
                          "{"
                          "    fColor = color;"                           // Pass color to fragment shader
                          "    fTexcoord = texcoord;"                     // Pass texcoord to fragment shader
                          "    gl_Position = projection * view * model * vec4(position, 1.0);"     // Place vertex at (x, y, z, 1) and then transform it according to the projection, view and model matrices
                          "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec3 fColor;"                              // From the vertex shader
                           "in vec2 fTexcoord;"                           // From the vertex shader
                           "uniform sampler2D diffuse;"                       // The texture
                           "out vec4 outputColor;"                        // The color of the resulting fragment
                           "void main()"
                           "{"
                           "    outputColor = texture(diffuse, fTexcoord)"   // Color using the color and texutre
                           "                  * vec4(fColor, 1.0);"
                           "}";

int main(void)
{
    GLFWwindow* window;

    // The OpenGL context creation code is in
    // ../common/util.cpp
    window = init("Hello Mesh", 640, 480);
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
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

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
    GLuint texture = loadImage("image.png", &w, &h, 0);
    if(!texture)
    {
        std::cerr << "Could not load texture" << std::endl;
        return -1;
    }
    mat.setDiffuseTexture(texture);

    // Load the mesh
    Mesh mesh;
    if(!mesh.load("test_mesh.obj"))
    {
        std::cerr << "Could not load mesh" << std::endl;
    }
    
    // Set the clear color to a light grey
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    float angle = 0.0f;

    while(!glfwWindowShouldClose(window))
    {
        // Clear (note the addition of GL_DEPTH_BUFFER_BIT)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate the mesh over time so we see the 3D effect better
        angle += glfwGetTime() * 2.0f;
        mesh.setAngle(angle, angle / 2.0f, 0.0f);
        glfwSetTime(0.0);

        const glm::mat4& model = mesh.getModelMatrix();

        // Upload the MVP matrices
        mat.bind();
        mat.setUniform("model", model);
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

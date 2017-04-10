#include "../common/util.h"
#include "../common/shader.h"
#include "material.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Vertex shading
const char* VERTEX_SRC_0 = "#version 330 core\n"
                         "layout(location=0) in vec3 position;"
                         "layout(location=1) in vec3 normal;"
                         "layout(location=2) in vec2 texcoord;"
                         "uniform mat4 model;"
                         "uniform mat4 view;"
                         "uniform mat4 projection;"
                         "uniform vec3 ambientLight;"
                         "uniform vec3 lightPos;"
                         "uniform vec3 lightColor;"
                         "flat out vec3 fColor;"
                         "out vec2 fTexcoord;"
                         "void main()"
                         "{"
                         "    fTexcoord = texcoord;"
                         "    vec3 normal_v = normalize(mat3(transpose(inverse(model))) * normal);"
                         "    vec3 ms_position = vec3(model * vec4(position, 1.0));"
                         "    vec3 lightDir = normalize(ms_position - lightPos);"
                         "    float diff = max(dot(normal_v, lightDir), 0.0);"
                         "    fColor = ambientLight + diff * lightColor;"
                         "    gl_Position = projection * view * model * vec4(position, 1.0);"
                         "}"
                         ;

const char* FRAGMENT_SRC_0 = "#version 330 core\n"
                           "flat in vec3 fColor;"
                           "in vec2 fTexcoord;"
                           "uniform sampler2D diffuse;"
                           "out vec4 outputColor;"
                           "void main()"
                           "{"
                           "    outputColor = texture(diffuse, fTexcoord) * vec4(fColor, 1.0);"
                           "}"
                           ;

// Fragment shading
const char* VERTEX_SRC_1 = "#version 330 core\n"
                         "layout(location=0) in vec3 position;"
                         "layout(location=1) in vec3 normal;"
                         "layout(location=2) in vec2 texcoord;"
                         "uniform mat4 model;"
                         "uniform mat4 view;"
                         "uniform mat4 projection;"
                         "out vec3 fNormal;"
                         "out vec2 fTexcoord;"
                         "out vec3 fPosition;"
                         "void main()"
                         "{"
                         "    fTexcoord = texcoord;"
                         "    fNormal = normalize(mat3(transpose(inverse(model))) * normal);"
                         "    fPosition = vec3(model * vec4(position, 1.0));"
                         "    gl_Position = projection * view * model * vec4(position, 1.0);"
                         "}"
                         ;

const char* FRAGMENT_SRC_1 = "#version 330 core\n"
                           "in vec3 fNormal;"
                           "in vec3 fPosition;"
                           "in vec2 fTexcoord;"
                           "uniform sampler2D diffuse;"
                           "uniform vec3 ambientLight;"
                           "uniform vec3 lightPos;"
                           "uniform vec3 lightColor;"
                           "out vec4 outputColor;"
                           "void main()"
                           "{"
                           "    vec3 lightDir = normalize(fPosition - lightPos);"
                           "    float diff = max(dot(fNormal, lightDir), 0.0);"
                           "    vec3 fColor = ambientLight + diff * lightColor;"
                           "    outputColor = texture(diffuse, fTexcoord) * vec4(fColor, 1.0);"
                           "}"
                           ;

int main(void)
{
    GLFWwindow* window;

    window = init("Vertex Shading", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)640/(float)480, 0.1f, 1000.0f);
    glm::mat4 view;

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

    Material mat0;
    if(!mat0.load(VERTEX_SRC_0, FRAGMENT_SRC_0))
    {
        std::cerr << "Could not load shaders" << std::endl;
        return -1;
    }
    Material mat1;
    if(!mat1.load(VERTEX_SRC_1, FRAGMENT_SRC_1))
    {
        std::cerr << "Could not load shaders" << std::endl;
        return -1;
    }
    int w, h;
    GLuint texture = loadImage("palette.png", &w, &h, 0, false);
    if(!texture)
    {
        std::cerr << "Could not load texture" << std::endl;
        return -1;
    }
    mat0.use();
    mat0.setDiffuseTexture(texture);
    mat1.use();
    mat1.setDiffuseTexture(texture);

    Mesh mesh;
    if(!mesh.load("monkey.obj"))
    {
        std::cerr << "Could not load mesh" << std::endl;
    }
    
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    float angle = 0.0f;

    glm::vec3 ambientLight(0.0, 0.0, 0.0);
    glm::vec3 lightPos(1.0f, 3.0f, -4.0f);
    glm::vec3 lightColor(1.0, 1.0, 1.0);

    bool flat = true;
    int previousState = GLFW_RELEASE;

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate the mesh over time so we see the 3D effect better
        angle += glfwGetTime();
        mesh.setAngle(angle, angle / 2.0f, 0.0f);
        glfwSetTime(0.0);

        int state = glfwGetKey(window, GLFW_KEY_E);
        if (state == GLFW_RELEASE && previousState == GLFW_PRESS)
        {
            flat = !flat;
            if (flat)
            {
                std::cout << "Vertex shading" << std::endl;
            }
            else
            {
                std::cout << "Fragment shading" << std::endl;
            }
        }
        previousState = state;

        const glm::mat4& model = mesh.getModelMatrix();

        if (!flat)
        {
            mat1.bind();
            mat1.setUniform("model", model);
            mat1.setUniform("view", view);
            mat1.setUniform("projection", proj);
            mat1.setUniform("ambientLight", ambientLight);
            mat1.setUniform("lightPos", lightPos);
            mat1.setUniform("lightColor", lightColor);
        }
        else
        {
            mat0.bind();
            mat0.setUniform("model", model);
            mat0.setUniform("view", view);
            mat0.setUniform("projection", proj);
            mat0.setUniform("ambientLight", ambientLight);
            mat0.setUniform("lightPos", lightPos);
            mat0.setUniform("lightColor", lightColor);
        }

        mesh.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

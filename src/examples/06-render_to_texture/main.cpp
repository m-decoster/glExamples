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

const char* VERTEX_FB_SRC = "#version 330 core\n"
                            "layout(location=0) in vec2 position;"
                            "layout(location=1) in vec2 texCoords;"
                            "out vec2 fTexCoords;"
                            "void main()"
                            "{"
                            "    gl_Position = vec4(position, 0.0f, 1.0f);"
                            "    fTexCoords = texCoords;"
                            "}";

const char* FRAGMENT_FB_SRC = "#version 330 core\n"
                              "in vec2 fTexCoords;"
                              "out vec4 outputColor;"
                              "uniform sampler2D diffuse;"
                              "void main()"
                              "{"
                              "    outputColor = texture(diffuse, fTexCoords);" // Google 'grayscale coefficients' if you don't understand the next line
                              "    float avg = 0.2126 * outputColor.r + 0.7152 * outputColor.g + 0.0722 * outputColor.b;"
                              "    outputColor = vec4(avg, avg, avg, 1.0);" // grayscale
                              "}";

int main(void)
{
    GLFWwindow* window;

    // The OpenGL context creation code is in
    // ../common/util.cpp
    window = init("Render to texture", 640, 480);
    if(!window)
    {
        return -1;
    }

    // Create the framebuffer object
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // Generate a texture for the framebuffer
    GLuint fbTexture;
    glGenTextures(1, &fbTexture);
    glBindTexture(GL_TEXTURE_2D, fbTexture);
    // The size of the texture is the size of the screen, and we send NULL as data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Now set the texture as the color buffer of the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);
    // Now generate a render buffer
    // We create this for a depth component so that OpenGL can do depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 640, 480);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Check if the framebuffer is complete. This should be the case after adding at least
    // one attachment (here we have added 2)
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Error creating framebuffer, it is incomplete!" << std::endl;
        return -1;
    }

    // Create a VAO for rendering the framebuffer
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint vbo;
    glGenBuffers(1, &vbo);

    float vertices[] =
    {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    // Create a perspective projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)640/(float)480, 0.1f, 1000.0f);
    // vertex_clip = M_projection . M_view . M_model . vertex_local
    // Create the view matrix
    glm::mat4 view;
    // Move the scene so that we can see the mesh
    view = glm::translate(view, glm::vec3(-2.0f, -1.5f, -8.0f));

    // Create a framebuffer material
    Material fbMat;
    if(!fbMat.load(VERTEX_FB_SRC, FRAGMENT_FB_SRC))
    {
        std::cerr << "Could not load framebuffer shaders" << std::endl;
        return -1;
    }

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
    GLuint texture = loadImage("image.png", &w, &h, 0, false);
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
    
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    float angle = 0.0f;
    while(!glfwWindowShouldClose(window))
    {
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

        // Draw the mesh to the framebuffer
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mesh.render();

        // Draw the framebuffer to the screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(vao);
        fbMat.setDiffuseTexture(fbTexture);
        fbMat.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Note how the mesh is now grayscale

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(1, &fbTexture);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwTerminate();
    return 0;
}

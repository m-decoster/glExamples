#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Z-Buffer Pre-Pass
// This avoids overdraw
// Whether you need this or not depends on your application and rendering pipeline
const char* VERTEX_Z_PASS_SRC = "#version 330 core\n"
                                "layout(location=0) in vec3 position;"
                                "uniform mat4 model;"
                                "uniform mat4 view;"
                                "uniform mat4 projection;"
                                "void main()"
                                "{"
                                "    gl_Position = projection * view * model * vec4(position, 1.0);"
                                "}";

const char* FRAGMENT_Z_PASS_SRC = "#version 330 core\n"
                                  "out vec4 color;"
                                  "void main()"
                                  "{"
                                  "    color = vec4(0.0, 0.0, 0.0, 1.0);" // Start out without light
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
                              "uniform sampler2D tex;"
                              "void main()"
                              "{"
                              "    outputColor = texture(tex, fTexCoords);"
                              "}";

int main(void)
{
    GLFWwindow* window;
    window = init("Additive Light Passes", 640, 480);
    if(!window)
    {
        return -1;
    }

    glm::mat4 model = glm::scale(glm::mat4(), glm::vec3(100.0f, 0.5f, 100.0f));
    model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    setCamera(&camera);

    GLuint zPassProgram, fbProgram;
    {
        GLuint vertex = createShader(VERTEX_Z_PASS_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_Z_PASS_SRC, GL_FRAGMENT_SHADER);
        zPassProgram = createShaderProgram(vertex, fragment);
        linkShader(zPassProgram);
        validateShader(zPassProgram);
        glDetachShader(zPassProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(zPassProgram, fragment);
        glDeleteShader(fragment);
    }
    {
        GLuint vertex = createShader(VERTEX_FB_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_FB_SRC, GL_FRAGMENT_SHADER);
        fbProgram = createShaderProgram(vertex, fragment);
        linkShader(fbProgram);
        validateShader(fbProgram);
        glDetachShader(fbProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(fbProgram, fragment);
        glDeleteShader(fragment);
    }
    GLuint vaoQuad;
    glGenVertexArrays(1, &vaoQuad);
    glBindVertexArray(vaoQuad);
    GLuint vboQuad;
    glGenBuffers(1, &vboQuad);

    float verticesQuad[] =
    {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesQuad), verticesQuad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    float vertices[] = {
        // x   y       z      norm                 u     v
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Framebuffer that will be used to render in multiple passes
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLuint fbTexture;
    glGenTextures(1, &fbTexture);
    glBindTexture(GL_TEXTURE_2D, fbTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 640, 480);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Error creating framebuffer, it is incomplete!" << std::endl;
        return -1;
    }

    int w, h;
    GLuint diffuse = loadImage("diffuseCube.png", &w, &h, 0, false);
    if(!diffuse)
    {
        return -1;
    }
    GLuint specular = loadImage("specularCube.png", &w, &h, 1, false);
    if(!specular)
    {
        return -1;
    }

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    
    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        updateCamera(640, 480, window);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);

        // 1. Z-PRE-PASS
        glDepthMask(GL_TRUE); // Do depth writing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(zPassProgram);
        glBindVertexArray(vao);
        // Render our floor
        glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
        glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getView()));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. LIGHTS
        glDepthMask(GL_FALSE); // Do not write depth anymore
        glUseProgram(lightPassProgram);
        for(int i = 0; i < 3; ++i)
        {
            // Send current framebuffer and write to the same buffer: don't do this!
            // OTHER SOLUTION NEEDED
            // Render the floor for each light
            // The vao is still bound
            glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
            glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getView()));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // DISPLAY
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // Do not perform depth testing
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(fbProgram);
        // Render the quad
        glBindVertexArray(vaoQuad);
        glActiveTexture(GL_TEXTURE1); // The framebuffer color buffer (fbTexture) is bound to GL_TEXTURE1
        glBindTexture(GL_TEXTURE_2D, fbTexture);
        glUniform1i(glGetUniformLocation(fbProgram, "tex"), 1); // The framebuffer color buffer (fbTexture) is bound to GL_TEXTURE1
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &diffuse);
    glDeleteTextures(1, &specular);
    glDeleteProgram(zPassProgram);

    glfwTerminate();
    return 0;
}

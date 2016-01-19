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
                                  "void main()"
                                  "{"
                                  "    gl_FragDepth = gl_FragCoord.z;"
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

    GLuint zPassProgram, lightProgram;
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
        GLuint vertex = createShader(VERTEX_LIGHT_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_LIGHT_SRC, GL_FRAGMENT_SHADER);
        lightProgram = createShaderProgram(vertex, fragment);
        linkShader(lightProgram);
        validateShader(lightProgram);
        glDetachShader(lightProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(lightProgram, fragment);
        glDeleteShader(fragment);
    }

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
    glGenFramebuffers(1, &depthMapFBO);  
    glGenTextures(1, &depthMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // Don't write to the color buffer
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
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

        // TODO: OPTIMIZATION
        // Scissor test using a screen space rectangle for each light

        // 2. LIGHTS
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_FALSE); // Do not write depth anymore
        glUseProgram(lightPassProgram);
        glUniform1i(glGetUniformLocation(lightPassProgram, "depthMap"), 1);
        for(int i = 0; i < 3; ++i)
        {
            // Render the floor for each light
            // The vao is still bound
            // TODO set light data in uniform
            glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
            glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getView()));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

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

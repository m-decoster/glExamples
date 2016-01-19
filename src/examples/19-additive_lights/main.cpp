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
                                "layout(location=1) in vec3 normal;"
                                "layout(location=2) in vec2 texCoords;"
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
                                  "}";

const char* VERTEX_LIGHT_SRC = "#version 330 core\n"
                               "layout(location=0) in vec3 position;"
                               "layout(location=1) in vec3 normal;"
                               "layout(location=2) in vec2 texCoords;"
                               "out vec3 fNormal;"
                               "out vec3 fPosition;"
                               "out vec2 fTexCoords;"
                               "uniform mat4 model;"
                               "uniform mat4 view;"
                               "uniform mat4 projection;"
                               "void main()"
                               "{"
                               "    gl_Position = projection * view * model * vec4(position, 1.0);"
                               "    fPosition = (model * vec4(position, 1.0)).xyz;"
                               "    fNormal = mat3(transpose(inverse(model))) * normal;"
                               "    fTexCoords = texCoords;"
                               "}";

const char* FRAGMENT_LIGHT_SRC = "#version 330 core\n"
                                  "in vec3 fNormal;"
                                  "in vec3 fPosition;"
                                  "in vec2 fTexCoords;"
                                  "out vec4 outputColor;"
                                  "uniform vec3 lightPosition;"
                                  "uniform vec3 lightColor;"
                                  "uniform vec3 lightAtt;"
                                  "uniform sampler2D tex;"
                                  "void main()"
                                  "{"
                                  "    float dist = length(lightPosition - fPosition);"
                                  "    float attenuation = 1.0f / (lightAtt.x + lightAtt.y * dist + lightAtt.z * dist * dist);"
                                  "    outputColor = vec4(lightColor * vec3(texture(tex, fTexCoords)) * attenuation, 1.0);"
                                  "}";

struct PointLight
{
    PointLight(const glm::vec3& pos, const glm::vec3& col, const glm::vec3& att)
        : position(pos), color(col), attenuation(att) {};
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 attenuation;
};

int main(void)
{
    GLFWwindow* window;
    window = init("Additive Light Passes", 640, 480);
    if(!window)
    {
        return -1;
    }

    glm::mat4 model = glm::scale(glm::mat4(), glm::vec3(100.0f, 1.0f, 100.0f));

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    setCamera(&camera);

    GLuint zPassProgram, lightPassProgram;
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
        lightPassProgram = createShaderProgram(vertex, fragment);
        linkShader(lightPassProgram);
        validateShader(lightPassProgram);
        glDetachShader(lightPassProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(lightPassProgram, fragment);
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

    int w, h;
    GLuint diffuse = loadImage("diffuseCube.png", &w, &h, 0, false);
    if(!diffuse)
    {
        return -1;
    }

    // attenuation
    glm::vec3 att(1.0f, 0.7f, 1.8f);
    PointLight redLight(glm::vec3(-0.25f, 0.0f, -0.25f), glm::vec3(1.0f, 0.0f, 0.0f), att);
    PointLight greenLight(glm::vec3(0.0f, 0.0f, 0.25f), glm::vec3(0.0f, 1.0f, 0.0f), att);
    PointLight blueLight(glm::vec3(0.25f, 0.0f, -0.25f), glm::vec3(0.0f, 0.0f, 1.0f), att);
    PointLight lights[3] = { redLight, greenLight, blueLight };

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    // Additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        updateCamera(640, 480, window);

        // 1. Z-PRE-PASS
        glDepthMask(GL_TRUE); // Do depth writing
        glDepthFunc(GL_LESS);
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
        glDepthFunc(GL_EQUAL);
        glUseProgram(lightPassProgram);
        glUniform1i(glGetUniformLocation(lightPassProgram, "tex"), 0); // diffuse texture is bound to GL_TEXTURE0
        glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
        glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getView()));
        for(int i = 0; i < 3; ++i)
        {
            // Render the floor for each light
            // Note that the vao is still bound from the z-pre-pass
            // All we do is set the uniforms and call glDrawArrays
            // Possible optimization: Scissor test using a screen space rectangle for each light (TODO)
            glUniform3fv(glGetUniformLocation(lightPassProgram, "lightPosition"), 1, glm::value_ptr(lights[i].position));
            glUniform3fv(glGetUniformLocation(lightPassProgram, "lightColor"), 1, glm::value_ptr(lights[i].color));
            glUniform3fv(glGetUniformLocation(lightPassProgram, "lightAtt"), 1, glm::value_ptr(lights[i].attenuation));
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
    glDeleteProgram(zPassProgram);

    glfwTerminate();
    return 0;
}

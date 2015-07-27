#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec3 position;"
                          "layout(location=1) in vec3 color;"
                          "layout(location=2) in vec2 texcoord;"
                          "uniform mat4 model;"
                          "uniform mat4 view;"
                          "uniform mat4 projection;"
                          "out vec3 fColor;"
                          "out vec2 fTexcoord;"
                          "void main()"
                          "{"
                          "    fColor = color;"
                          "    fTexcoord = texcoord;"
                          "    gl_Position = projection * view * model * vec4(position, 1.0);"
                          "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec3 fColor;"
                           "in vec2 fTexcoord;"
                           "uniform sampler2D tex;"
                           "out vec4 outputColor;"
                           "void main()"
                           "{"
                           "    outputColor = texture(tex, fTexcoord)"
                           "                  * vec4(fColor, 1.0);"
                           "}";

const char* VERTEX_BB_SRC = "#version 330 core\n"
                            "layout(location=0) in vec2 position;"
                            "uniform vec3 center;"
                            "uniform vec2 size;"
                            "uniform mat4 view;"
                            "uniform mat4 proj;"
                            "out vec2 fTexCoords;"
                            "void main()"
                            "{"
                            "    vec3 vertexPos = center;"
                            "    gl_Position = proj * view * vec4(vertexPos, 1.0);"
                            "    gl_Position /= gl_Position.w;"
                            "    gl_Position.xy += position.xy * size;"
                            "    fTexCoords = position + vec2(0.5, 0.5);"
                            "}";

const char* FRAGMENT_BB_SRC = "#version 330 core\n"
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

    window = init("Billboards", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    camera.setPosition(0.0f, 0.0f, -3.0f);
    setCamera(&camera); // The camera updating is handled in ../common/util.cpp

    glm::mat4 model;

    GLuint program;
    {
        GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
        program = createShaderProgram(vertex, fragment);
        linkShader(program);
        validateShader(program);
        glDetachShader(program, vertex);
        glDeleteShader(vertex);
        glDetachShader(program, fragment);
        glDeleteShader(fragment);
    }
    GLuint billboardProgram;
    {
        GLuint vertex = createShader(VERTEX_BB_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_BB_SRC, GL_FRAGMENT_SHADER);
        billboardProgram = createShaderProgram(vertex, fragment);
        linkShader(billboardProgram);
        validateShader(billboardProgram);
        glDetachShader(program, vertex);
        glDeleteShader(vertex);
        glDetachShader(program, fragment);
        glDeleteShader(fragment);
    }

    glUseProgram(program);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    float vertices[] =
    {
        // x   y      z     r     g     b     u     v
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2); // texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    float verts_bb[] =
    {
        -0.5f, -0.5f,
        0.5f,  -0.5f,
        -0.5f, 0.5f,
        0.5f, 0.5f
    };
    GLuint vbo_bb;
    glGenBuffers(1, &vbo_bb);
    GLuint vao_bb;
    glGenVertexArrays(1, &vao_bb);
    glBindVertexArray(vao_bb);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_bb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts_bb), verts_bb, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUseProgram(billboardProgram);
    glUniform3f(glGetUniformLocation(billboardProgram, "center"), 0.0f, 2.0f, 0.0f);
    glUniform2f(glGetUniformLocation(billboardProgram, "size"), 0.5f, 0.5f);

    int w, h;
    GLuint texture = loadImage("image.png", &w, &h, 0, false);
    glUniform1i(glGetUniformLocation(billboardProgram, "tex"), 0);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateCamera(640, 480, window);

        // Draw the cube
        glUseProgram(program);
        glBindVertexArray(vao);

        GLint modelUL = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(modelUL, 1, GL_FALSE, glm::value_ptr(model));
        GLint viewUL = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewUL, 1, GL_FALSE, glm::value_ptr(camera.getView()));
        GLint projUL = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projUL, 1, GL_FALSE, glm::value_ptr(camera.getProjection()));

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw the billboard
        glBindVertexArray(vao_bb);
        glUseProgram(billboardProgram);

        viewUL = glGetUniformLocation(billboardProgram, "view");
        glUniformMatrix4fv(viewUL, 1, GL_FALSE, glm::value_ptr(camera.getView()));
        projUL = glGetUniformLocation(billboardProgram, "proj");
        glUniformMatrix4fv(projUL, 1, GL_FALSE, glm::value_ptr(camera.getProjection()));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo_bb);
    glDeleteVertexArrays(1, &vao_bb);
    glDeleteProgram(program);
    glDeleteProgram(billboardProgram);
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

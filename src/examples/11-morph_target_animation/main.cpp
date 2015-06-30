#include "../common/util.h"
#include "../common/shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#define CUR_POS_VBO 0
#define NEXT_POS_VBO 1
#define TBO 2

#define FRAME_TIME 3.0f

const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec3 curPos;" // current frame position of vertices
                         "layout(location=1) in vec3 nextPos;" // next frame position of vertices
                         "uniform mat4 model;"
                         "uniform mat4 view;"
                         "uniform mat4 projection;"
                         "uniform float tween;" // used for linear interpolation
                         "out vec2 fTexcoord;"
                         "void main()"
                         "{"
                         "    gl_Position = vec4(mix(curPos, nextPos, tween), 1.0);" // linear interpolation
                         "    gl_Position = projection * view * model * gl_Position;"
                         "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "out vec4 outputColor;"
                           "void main()"
                           "{"
                           "    outputColor = vec4(1.0);"
                           "}";

int main(void)
{
    GLFWwindow* window = init("Morph Target Animation", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -4.0f));
    glm::mat4 model = glm::rotate(glm::mat4(), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f));

    // Load the shader
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

        if(!program)
        {
            return -1;
        }
    }
    glUseProgram(program);

    float vertices1[] =
    {
        // x   y      z
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
    
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
    
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
    
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
    
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
    
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };
    float vertices2[] =
    {
        // x   y      z     
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
    
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
    
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
    
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
    
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
    
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
    };

    // Create the vao and buffer objects
    GLuint vao;
    GLuint buffers[2];
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(2, buffers);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[CUR_POS_VBO]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[NEXT_POS_VBO]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    float timeSinceFrame = 0.0f;
    int currentFrame;
    glBindVertexArray(vao);
    while(!glfwWindowShouldClose(window))
    {
        // If the time for a frame has passed, we have to switch frames
        timeSinceFrame += glfwGetTime();
        if(timeSinceFrame >= FRAME_TIME)
        {
            if(currentFrame == 0)
            {
                currentFrame = 1;
            }
            else
            {
                currentFrame = 0;
            }
            timeSinceFrame = 0.0f;

            // Update the buffers
            glBindBuffer(GL_ARRAY_BUFFER, buffers[CUR_POS_VBO]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), 
                    currentFrame == 0 ? vertices1 : vertices2, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, buffers[NEXT_POS_VBO]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1),
                    currentFrame == 0 ? vertices2 : vertices1, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1f(glGetUniformLocation(program, "tween"), timeSinceFrame / FRAME_TIME);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSetTime(0.0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}

#include "../common/util.h"
#include "../common/shader.h"
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

int main(void)
{
    GLFWwindow* window;

    window = init("Transparency", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)640/(float)480, 0.1f, 1000.0f);
    glm::mat4 model1, model2, model3; // Cube 1 and 2 will be transparent, cube 3 not
    model1 = glm::translate(model1, glm::vec3(0.5f, 0.0f, -1.0f));
    model2 = glm::translate(model2, glm::vec3(0.0f, 0.5f, -2.0f));
    model3 = glm::translate(model3, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 view;
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

    GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
    if(!vertex)
    {
        return -1;
    }
    GLuint fragment = createShader(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
    if(!fragment)
    {
        return -1;
    }
    GLuint program = createShaderProgram(vertex, fragment);
    if(!program)
    {
        return -1;
    }
    bool result = linkShader(program);
    if(!result)
    {
        return -1;
    }
    result = validateShader(program);
    if(!result)
    {
        return -1;
    }
    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);

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

    int w, h;
    GLuint textureTrans = loadImage("transparent.png", &w, &h, 0, true); // GL_TEXTURE0
    if(!textureTrans)
    {
        return -1;
    }
    GLuint textureOpaq = loadImage("opaque.png", &w, &h, 0, true); // GL_TEXTURE0
    if(!textureOpaq)
    {
        return -1;
    }
    glUniform1i(glGetUniformLocation(program, "tex"), 0); // GL_TEXTURE0

    GLint viewUL = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewUL, 1, GL_FALSE, glm::value_ptr(view));
    GLint projUL = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projUL, 1, GL_FALSE, glm::value_ptr(proj));
    GLint modelUL = glGetUniformLocation(program, "model");
    
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        // Clear (note the addition of GL_DEPTH_BUFFER_BIT)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // We have to sort the objects, rendering the backmost objects first
        // in this example, model1 is in front of model2, which is in front of model3
        // and model3 is the only one using the opaque texture
        // (This way we can avoid the sorting, making the example a bit easier to follow)
        glBindTexture(GL_TEXTURE_2D, textureOpaq);
        glUniformMatrix4fv(modelUL, 1, GL_FALSE, glm::value_ptr(model3));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindTexture(GL_TEXTURE_2D, textureTrans);
        glUniformMatrix4fv(modelUL, 1, GL_FALSE, glm::value_ptr(model2));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glUniformMatrix4fv(modelUL, 1, GL_FALSE, glm::value_ptr(model1));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &textureTrans);
    glDeleteTextures(1, &textureOpaq);

    glfwTerminate();
    return 0;
}

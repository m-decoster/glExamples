#include "../common/util.h"
#include "../common/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec3 position;"          // Vertex position (x, y, z)
                          "layout(location=1) in vec3 color;"             // Vertex color (r, g, b)
                          "layout(location=2) in vec2 texcoord;"          // Texture coordinate (u, v)
                          "uniform mat4 model;"
                          "layout(std140) uniform PV"                     // Uniform block
                          "{"
                          "    mat4 projection;"
                          "    mat4 view;"
                          "};"
                          "out vec3 fColor;"                              // Vertex shader has to pass color to fragment shader
                          "out vec2 fTexcoord;"                           // Pass to fragment shader
                          "void main()"
                          "{"
                          "    fColor = color;"                           // Pass color to fragment shader
                          "    fTexcoord = texcoord;"                     // Pass texcoord to fragment shader
                          "    gl_Position = projection * view * model * vec4(position, 1.0);"     // Place vertex at (x, y, z, 1) and then transform it according to the projection, view and model matrices
                          "}";

const char* FRAGMENT_1_SRC = "#version 330 core\n"
                             "in vec3 fColor;"                              // From the vertex shader
                             "in vec2 fTexcoord;"                           // From the vertex shader
                             "uniform sampler2D tex;"                       // The texture
                             "out vec4 outputColor;"                        // The color of the resulting fragment
                             "void main()"
                             "{"
                             "    outputColor = texture(tex, fTexcoord)"   // Color using the color and texutre
                             "                  * vec4(fColor, 1.0);"
                             "}";

const char* FRAGMENT_2_SRC = "#version 330 core\n"
                             "in vec3 fColor;"                              // From the vertex shader
                             "in vec2 fTexcoord;"                           // From the vertex shader
                             "uniform sampler2D tex;"                       // The texture
                             "out vec4 outputColor;"                        // The color of the resulting fragment
                             "void main()"
                             "{"
                             "    outputColor = texture(tex, fTexcoord)"   // Color using the color and texutre
                             "                  * vec4(fColor, 1.0);"
                             "    float avg = 0.2126 * outputColor.r + 0.7152 * outputColor.g + 0.0722 * outputColor.b;"
                             "    outputColor = vec4(avg, avg, avg, 1.0);" // Greyscale
                             "}";

int main(void)
{
    GLFWwindow* window;

    window = init("Uniform Buffer Objects", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 1000.0f);
    glm::mat4 model1 = glm::translate(glm::mat4(), glm::vec3(-1.0f, 0.0f, 0.0f));
    glm::mat4 model2 = glm::translate(glm::mat4(), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -4.0f));

    // We create the two shader programs. They reuse the same vertex shader
    GLuint program1, program2;
    {
        GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
        GLuint fragment1 = createShader(FRAGMENT_1_SRC, GL_FRAGMENT_SHADER);
        GLuint fragment2 = createShader(FRAGMENT_2_SRC, GL_FRAGMENT_SHADER);
        program1 = createShaderProgram(vertex, fragment1);
        program2 = createShaderProgram(vertex, fragment2);
        linkShader(program1);
        linkShader(program2);
        validateShader(program1);
        validateShader(program2);

        glDetachShader(program1, vertex);
        glDetachShader(program2, vertex);
        glDeleteShader(vertex);
        glDetachShader(program1, fragment1);
        glDetachShader(program2, fragment2);
        glDeleteShader(fragment1);
        glDeleteShader(fragment2);
    }

    // We now create the data to send to the GPU
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    float vertices[] =
    {
        // x   y      z      r     g     b     u     v
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

    // NEW: Create the uniform buffer object
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); // pre-allocate

    // Bind the uniform blocks to indices for each shader program
    GLuint pv1_index = glGetUniformBlockIndex(program1, "PV");   
    glUniformBlockBinding(program1, pv1_index, 0);
    GLuint pv2_index = glGetUniformBlockIndex(program2, "PV");
    glUniformBlockBinding(program2, pv2_index, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo); // The ubo is now linked to 0 for both shaders
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Load the texture and bind it to the uniform
    int w, h;
    GLuint texture = loadImage("image.png", &w, &h, 0, false); // GL_TEXTURE0
    if(!texture)
    {
        return -1;
    }
    glUseProgram(program1);
    glUniform1i(glGetUniformLocation(program1, "tex"), 0); // GL_TEXTURE0
    glUseProgram(program2);
    glUniform1i(glGetUniformLocation(program2, "tex"), 0); // GL_TEXTURE0
    
    // Set the clear color to a light grey
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    // The projection matrix doesn't change in this example, so we can upload it in advance
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Now upload the view part of the ubo
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) /* offset */, sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindVertexArray(vao);

        // Draw cube 1
        glUseProgram(program1);
        glUniformMatrix4fv(glGetUniformLocation(program1, "model"), 1, GL_FALSE, glm::value_ptr(model1));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Draw cube 2
        glUseProgram(program2);
        glUniformMatrix4fv(glGetUniformLocation(program2, "model"), 1, GL_FALSE, glm::value_ptr(model2));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ubo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);
    glDeleteProgram(program1);
    glDeleteProgram(program2);

    glfwTerminate();
    return 0;
}

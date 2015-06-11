#include "../common/util.h"
#include "../common/shader.h"

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec2 position;"          // Vertex position (x, y)
                          "layout(location=1) in vec3 color;"             // Vertex color (r, g, b)
                          "layout(location=2) in vec2 texcoord;"          // Texture coordinate (u, v)
                          "out vec3 fColor;"                              // Vertex shader has to pass color to fragment shader
                          "out vec2 fTexcoord;"                           // Pass to fragment shader
                          "void main()"
                          "{"
                          "    fColor = color;"                           // Pass color to fragment shader
                          "    fTexcoord = texcoord;"                     // Pass texcoord to fragment shader
                          "    gl_Position=vec4(position, 0.0, 1.0);"     // Place vertex at (x, y, 0, 1)
                          "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec3 fColor;"                              // From the vertex shader
                           "in vec2 fTexcoord;"                           // From the vertex shader
                           "uniform sampler2D tex;"                       // The texture
                           "out vec4 outputColor;"                        // The color of the resulting fragment
                           "void main()"
                           "{"
                           "    outputColor = texture(tex, fTexcoord)"   // Color using the color and texutre
                           "                  * vec4(fColor, 1.0);"
                           "}";

int main(void)
{
    GLFWwindow* window;

    // The OpenGL context creation code is in
    // ../common/util.cpp
    window = init("Hello Sprite", 640, 480);
    if(!window)
    {
        return -1;
    }

    // We start by creating a vertex and fragment shader
    // from the above strings
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
    // Now we must make a shader program: this program
    // contains both the vertex and the fragment shader
    GLuint program = createShaderProgram(vertex, fragment);
    if(!program)
    {
        return -1;
    }
    // We link the program, just like your C compiler links
    // .o files
    bool result = linkShader(program);
    if(!result)
    {
        return -1;
    }
    // We make sure the shader is validated
    result = validateShader(program);
    if(!result)
    {
        return -1;
    }
    // Detach and delete the shaders, because we no longer need them
    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);

    glUseProgram(program); // Set this as the current shader program

    // We now create the data to send to the GPU
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    float vertices[] =
    {
        // x   y      r     g     b     u     v
        -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    // Upload the vertices to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    // Upload the indices (elements) to the buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Enable the vertex attributes and upload their data (see: layout(location=x))
    glEnableVertexAttribArray(0); // position
    // 2 floats: x and y, but 7 floats in total per row
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1); // color
    // 3 floats: r, g and b, but 7 floats in total per row and start at the third one
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2); // texture coordinates
    // 2 floats: u and v, but 7 floats in total per row and start at the sixth one
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

    // We have now successfully created a drawable Vertex Array Object

    // Load the texture and bind it to the uniform
    int w, h;
    GLuint texture = loadImage("image.png", &w, &h, 0); // GL_TEXTURE0
    if(!texture)
    {
        return -1;
    }
    glUniform1i(glGetUniformLocation(program, "tex"), 0); // GL_TEXTURE0
    
    // Set the clear color to a light grey
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        // Clear
        glClear(GL_COLOR_BUFFER_BIT);

        // The VAO is still bound so just draw the 6 vertices
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Tip: if nothing is drawn, check the return value of glGetError and google it

        // Swap buffers to show current image on screen (for more information google 'backbuffer')
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}

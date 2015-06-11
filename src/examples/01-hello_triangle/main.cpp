#include "../common/util.h"
#include "../common/shader.h"

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec2 position;"          // Vertex position (x, y)
                          "layout(location=1) in vec3 color;"             // Vertex color (r, g, b)
                          "out vec3 fColor;"                              // Vertex shader has to pass color to fragment shader
                          "void main()"
                          "{"
                          "    fColor = color;"                           // Pass color to fragment shader
                          "    gl_Position=vec4(position, 0.0, 1.0);"     // Place vertex at (x, y, 0, 1)
                          "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec3 fColor;"                              // From the vertex shader
                           "out vec4 outputColor;"                        // The color of the resulting fragment
                           "void main()"
                           "{"
                           "    outputColor = vec4(fColor, 1.0);"         // Color it (r, g, b, 1.0) for fully opaque
                           "}";

int main(void)
{
    GLFWwindow* window;

    // The OpenGL context creation code is in
    // ../common/util.cpp
    window = init("Hello Triangle", 640, 480);
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

    glUseProgram(program); // Set this as the current shader program

    // We now create the data to send to the GPU
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    float vertices[] =
    {
        // x   y     r     g     b
        -0.5f, 0.5f,  0.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
    };

    // Upload the vertices to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Enable the vertex attributes and upload their data (see: layout(location=x))
    glEnableVertexAttribArray(0); // position
    // 2 floats: x and y, but 5 floats in total per row
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1); // color
    // 3 floats: r, g and b, but 5 floats in total per row and start at the third one
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    // We have now successfully created a drawable Vertex Array Object
    
    // Set the clear color to a light grey
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        // Clear
        glClear(GL_COLOR_BUFFER_BIT);

        // The VAO is still bound so just draw the 3 vertices
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Tip: if nothing is drawn, check the return value of glGetError and google it

        // Swap buffers to show current image on screen (for more information google 'backbuffer')
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

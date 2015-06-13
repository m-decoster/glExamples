#include "../common/util.h"
#include "../common/shader.h"
#include "heightmap.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec3 position;"          // Vertex position (x, y, z)
                          "layout(location=1) in vec3 color;"             // Vertex color (r, g, b)
                          "out vec3 fColor;"                              // Vertex shader has to pass color to fragment shader
                          "uniform mat4 model;"
                          "uniform mat4 view;"
                          "uniform mat4 projection;"
                          "void main()"
                          "{"
                          "    fColor = color;"                           // Pass color to fragment shader
                          "    gl_Position = projection * view * model * vec4(position, 1.0);"     // Place vertex at (x, y, z, 1) and then transform it according to the projection, view and model matrices
                          "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec3 fColor;"                              // From the vertex shader
                           "out vec4 outputColor;"                        // The color of the resulting fragment
                           "void main()"
                           "{"
                           "    outputColor = vec4(fColor, 1.0);"         // Color it (r, g, b, 1.0) for fully opaque
                           "}";

const int TILE_WIDTH = 10;

int main(void)
{
    GLFWwindow* window;

    // The OpenGL context creation code is in
    // ../common/util.cpp
    window = init("Hello Heightmap", 640, 480);
    if(!window)
    {
        return -1;
    }

    // We will need to enable depth testing, so that OpenGL draws further
    // vertices first
    glEnable(GL_DEPTH_TEST);

    // Create a perspective projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)640/(float)480, 0.1f, 1000.0f);
    // vertex_clip = M_projection . M_view . M_model . vertex_local
    // Create the model matrix
    glm::mat4 model;
    // Create the view matrix
    glm::mat4 view;
    // Move the scene so that we can see the heightmap
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

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

    // Load the heightmap
    HeightMap map(2.0f);
    if(!map.load("heightmap.bmp"))
    {
        return -1;
    }
    const std::vector<float>& data = map.getData();
    std::vector<float> vertices;
    std::vector<GLuint> indices;
    for(int i = 0; i < map.getWidth(); ++i)
    {
        for(int j = 0; j < map.getHeight(); ++j)
        {
            // data[i * w + j] contains the height at that tile
            // we need to create 4 vertices per tile

            float tileHeight = data.at(i * map.getWidth() + j);
            
            // Top left
            vertices.push_back(i * TILE_WIDTH); // x
            vertices.push_back(tileHeight); // y
            vertices.push_back(j * TILE_WIDTH); // z
            for(int k = 0; k < 3; ++k)
            {
                vertices.push_back(0.0f); // r g b
            }
            // Top right
            vertices.push_back(i * TILE_WIDTH + TILE_WIDTH);
            vertices.push_back(tileHeight);
            vertices.push_back(j * TILE_WIDTH);
            for(int k = 0; k < 3; ++k)
            {
                vertices.push_back(0.0f); // r g b
            }
            // Bottom right
            vertices.push_back(i * TILE_WIDTH + TILE_WIDTH);
            vertices.push_back(tileHeight);
            vertices.push_back(j * TILE_WIDTH + TILE_WIDTH);
            for(int k = 0; k < 3; ++k)
            {
                vertices.push_back(0.0f); // r g b
            }
            // Bottom left
            vertices.push_back(i * TILE_WIDTH);
            vertices.push_back(tileHeight);
            vertices.push_back(j * TILE_WIDTH + TILE_WIDTH);
            for(int k = 0; k < 3; ++k)
            {
                vertices.push_back(0.0f); // r g b
            }

            // Indices (note that this is not optimal, because
            // vertices can be shared between tiles)
            int offset = 4 * i * j;
            indices.push_back(0 + offset);
            indices.push_back(1 + offset);
            indices.push_back(2 + offset);
            indices.push_back(0 + offset);
            indices.push_back(2 + offset);
            indices.push_back(3 + offset);
        }
    }

    // We now create the data to send to the GPU
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    // Upload the vertices to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Enable the vertex attributes and upload their data (see: layout(location=x))
    glEnableVertexAttribArray(0); // position
    // 3 floats: x, y and z, but 6 floats in total per row
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1); // color
    // 3 floats: r, g and b, but 3 floats in total per row and start at the 4th one
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // We have now successfully created a drawable Vertex Array Object
    
    // Set the clear color to a light grey
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        // Clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Upload the MVP matrices
        GLint modelUL = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(modelUL, 1, GL_FALSE, glm::value_ptr(model));
        GLint viewUL = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewUL, 1, GL_FALSE, glm::value_ptr(view));
        // This can be moved out of the loop because 
        GLint projUL = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projUL, 1, GL_FALSE, glm::value_ptr(proj));

        // The VAO is still bound so just draw the vertices
        // We use GL_LINES to draw as wireframe (you wouldn't see any difference in height
        // due to the lack of shading)
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Tip: if nothing is drawn, check the return value of glGetError and google it

        // Swap buffers to show current image on screen (for more information google 'backbuffer')
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    glfwTerminate();
    return 0;
}

#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include "heightmap.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

static glm::mat4 model;

static const float SPEED = 50.0f; // Camera transform speed
static const int SIZE = 10.0f; // Size of a single tile in the heightmap
static const float MOUSE_SPEED = 0.025f;

const char* VERTEX_SRC = "#version 330 core\n"
                          "layout(location=0) in vec3 position;"          // Vertex position (x, y, z)
                          "uniform mat4 model;"
                          "uniform mat4 view;"
                          "uniform mat4 projection;"
                          "void main()"
                          "{"
                          "    gl_Position = projection * view * model * vec4(position, 1.0);"     // Place vertex at (x, y, z, 1) and then transform it according to the projection, view and model matrices
                          "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "uniform sampler2D tex;"                       // The texture
                           "out vec4 outputColor;"                        // The color of the resulting fragment
                           "void main()"
                           "{"
                           "    outputColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);"
                           "}";

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

    // Hide the cursor (escape will exit the application)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // We will need to enable depth testing, so that OpenGL draws further
    // vertices first
    glEnable(GL_DEPTH_TEST);

    // Draw wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Enable backface culling
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // front facing vertices are defined counter clock wise

    // Create the model matrix
    model = glm::mat4();
    // Rotate just a bit (the vector indicates the axes on which to rotate)
    model = glm::rotate(model, -glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 1.0f));

    // Set the camera
    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    camera.setPosition(0.0f, 0.0f, -3.0f);

    // Load the heightmap
    HeightMap map(20.0f);
    if(!map.load("heightmap.bmp"))
    {
        return -1;
    }
    const std::vector<float>& data = map.getData();
    int w = map.getWidth(), h = map.getHeight();
    std::vector<float> vertices;
    for(int i = 0; i < h; ++i)
    {
        for(int j = 0; j < w; ++j)
        {
            float x = (float)i;
            float z = (float)j;
            float height = data[i * w + j];

            vertices.push_back(x * SIZE);
            vertices.push_back(height);
            vertices.push_back(z * SIZE);
            // This is where you could add extra information,
            // like colour or texture coordinates.
            // You would have to change the vertex attribute pointer
            // code as well!
        }
    }
    std::vector<GLuint> indices;
    for(int i = 0; i < (h - 1); ++i)
    {
        for(int j = 0; j < (w - 1); ++j)
        {
            // We create six indices for each tile
            indices.push_back(i * w + j);
            indices.push_back((i + 1) * w + j);
            indices.push_back(i * w + j + 1);
            indices.push_back(i * w + j + 1);
            indices.push_back((i + 1) * w + j);
            indices.push_back((i + 1) * w + j + 1);
        }
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

    // Upload the vertices to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    // Upload the indices to the buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // Enable the vertex attributes and upload their data (see: layout(location=x))
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    // We have now successfully created a drawable Vertex Array Object

    // Set the clear color to a light grey
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        float deltaTime = (float)glfwGetTime();
        glfwSetTime(0.0);

        // Get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwSetCursorPos(window, 320, 240);
        float horizontalAngle = camera.getHorizontalAngle();
        float verticalAngle = camera.getVerticalAngle();
        horizontalAngle += MOUSE_SPEED * deltaTime * (float)(320 - xpos);
        verticalAngle   += MOUSE_SPEED * deltaTime * (float)(240 - ypos);
        camera.setHorizontalAngle(horizontalAngle);
        camera.setVerticalAngle(verticalAngle);

        // Get key input
        glm::vec3 direction = camera.getDirectionVector();
        glm::vec3 position = camera.getPosition();
        glm::vec3 right = camera.getRightVector();
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            position += direction * deltaTime * SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            position -= direction * deltaTime * SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            position -= right * deltaTime * SPEED;
        }
        else if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            position += right * deltaTime * SPEED;
        }
        camera.setPosition(position.x, position.y, position.z);

        // Clear (note the addition of GL_DEPTH_BUFFER_BIT)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Upload the MVP matrices
        GLint modelUL = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(modelUL, 1, GL_FALSE, glm::value_ptr(model));
        GLint viewUL = glGetUniformLocation(program, "view");
        glUniformMatrix4fv(viewUL, 1, GL_FALSE, glm::value_ptr(camera.getView()));
        // This can be moved out of the loop because it rarely changes
        GLint projUL = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projUL, 1, GL_FALSE, glm::value_ptr(camera.getProjection()));

        // The VAO is still bound so just draw the vertices
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

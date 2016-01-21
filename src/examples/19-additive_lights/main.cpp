#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

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
                                  "void main()"
                                  "{"
                                  "    float dist = length(lightPosition - fPosition);"
                                  "    float attenuation = 1.0f / (lightAtt.x + lightAtt.y * dist + lightAtt.z * dist * dist);"
                                  "    outputColor = vec4(lightColor * attenuation, 1.0);"
                                  "}";

const char* VERTEX_STENCIL_SRC = "#version 330 core\n"
                                 "in vec2 position;"
                                 "void main()"
                                 "{"
                                 "    gl_Position = vec4(position, 1.0, 1.0);"
                                 "}";

const char* FRAGMENT_STENCIL_SRC = "#version 330 core\n"
                                   "out vec4 outputColor;"
                                   "void main()"
                                   "{"
                                   "    outputColor = vec4(1.0);"
                                   "}";

struct PointLight
{
    PointLight(const glm::vec3& pos, const glm::vec3& col, const glm::vec3& att)
        : position(pos), color(col), attenuation(att) {};
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 attenuation;
};

// Don't actually draw quads like this
void drawQuad(GLuint program, int* boundingBox, int width, int height)
{
    static bool firstRun = true;
    static GLuint vao = 0;
    if(firstRun)
    {
        glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);
    static GLuint vbo = 0;
    if(firstRun)
    {
        glGenBuffers(1, &vbo);
    }
    // Oh boy
    float vertices[] = {
        (float)boundingBox[0] / (width / 2) - 1, (float)boundingBox[1] / (height / 2) - 1, // top left
        (float)boundingBox[2] / (width / 2) - 1, (float)boundingBox[1] / (height / 2) - 1, // top right
        (float)boundingBox[0] / (width / 2) - 1, (float)boundingBox[3] / (height / 2) - 1, // bottom left
        (float)boundingBox[0] / (width / 2) - 1, (float)boundingBox[3] / (height / 2) - 1, // bottom left
        (float)boundingBox[2] / (width / 2) - 1, (float)boundingBox[1] / (height / 2) - 1, // top right
        (float)boundingBox[2] / (width / 2) - 1, (float)boundingBox[3] / (height / 2) - 1, // bottom right
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    if(firstRun)
    {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
    glBindVertexArray(0);
    firstRun = false;
}

// Courtesy of http://learnopengl.com/#!Advanced-Lighting/Deferred-Shading
// Calculate a light's radius based on its attenuation
float lightRadius(const PointLight& light)
{
    float maxComponent = std::fmaxf(std::fmaxf(light.color.x, light.color.y), light.color.z);
    float constant = light.attenuation.x;
    float linear = light.attenuation.y;
    float quadratic = light.attenuation.z;
    float radius = (-linear + std::sqrtf(linear * linear - 4.0f * quadratic * (constant - (256.0f / 2.0f) * maxComponent))) / (2.0f * quadratic);
    return radius;
}

// Calculate a light's 3D bounding box based on its radius and position
std::vector<glm::vec3> lightBB(const PointLight& light)
{
    std::vector<glm::vec3> result;
    result.reserve(8);
    float radius = lightRadius(light);
    float diameter = 2.0f * radius;

    // TOP
    glm::vec3 topLeftBack = light.position;
              topLeftBack.x -= radius;
              topLeftBack.y += radius;
              topLeftBack.z += radius;
    glm::vec3 topRightBack = topLeftBack;
              topRightBack.x += diameter;
    glm::vec3 topLeftFront = topLeftBack;
              topLeftFront.z -= diameter;
    glm::vec3 topRightFront = topLeftFront;
              topRightFront.x += diameter;
    // BOTTOM
    glm::vec3 bottomLeftBack = topLeftBack;
              bottomLeftBack.y -= diameter;
    glm::vec3 bottomRightBack = bottomLeftBack;
              bottomRightBack.x += diameter;
    glm::vec3 bottomLeftFront = bottomLeftBack;
              bottomLeftFront.z -= diameter;
    glm::vec3 bottomRightFront = bottomLeftFront;
              bottomRightFront.x += diameter;
    
    result.push_back(topLeftBack);
    result.push_back(topRightBack);
    result.push_back(topLeftFront);
    result.push_back(topRightFront);

    result.push_back(bottomLeftBack);
    result.push_back(bottomRightBack);
    result.push_back(bottomLeftFront);
    result.push_back(bottomRightFront);

    return result;
}

// Calculate a light's bounding box in screen space based on its bounding box in world space
void lightBBScreen(const PointLight &light, const glm::mat4& proj, const glm::mat4& view, int width, int height, int* result)
{
    std::vector<glm::vec3> worldBB = lightBB(light);
    int minX = width, maxX = 0, minY = height, maxY = 0;
    for(auto it = worldBB.begin(); it != worldBB.end(); ++it)
    {
        glm::vec4 v = glm::vec4(it->x, it->y, it->z, 1.0);
        v = proj * (view * v);
        if (v.w <= 0.0)
        {
            // We are very close to the light source...
            // Using the scissors here would generate artifacts.
            // Because we are close, we can just render the entire screen.
            minX = 0;
            minY = 0;
            maxX = width;
            maxY = height;
            break;
        }
        glm::vec3 norm_dev_coord_v = glm::vec3(v) / v.w;
        // [-1,1] and [-1,1] -> [0,width] and [0,height]
        int x = static_cast<int> (((norm_dev_coord_v.x + 1.0f) / 2.0f) * width);
        int y = static_cast<int> (((norm_dev_coord_v.y + 1.0f) / 2.0f) * height);
        if (x < minX && x >= 0)       minX = x;
        if (x > maxX && x <= width)   maxX = x;
        if (y < minY && y >= 0)       minY = y;
        if (y > maxY && y <= height)  maxY = y;
    }
    result[0] = minX;
    result[1] = minY;
    result[2] = maxX;
    result[3] = maxY;
}

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

    GLuint zPassProgram, lightPassProgram, stencilProgram;
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
    {
        GLuint vertex = createShader(VERTEX_STENCIL_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_STENCIL_SRC, GL_FRAGMENT_SHADER);
        stencilProgram = createShaderProgram(vertex, fragment);
        linkShader(stencilProgram);
        validateShader(stencilProgram);
        glDetachShader(stencilProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(stencilProgram, fragment);
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

    // attenuation
    glm::vec3 att(1.0f, 0.35f, 0.44f);
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

        int width, height;
        int boundingBox[4]; // for each light
        glfwGetFramebufferSize(window, &width, &height);

        // 1. Z-PRE-PASS
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write colors
        glDepthMask(GL_TRUE); // Do depth writing
        glDepthFunc(GL_LESS);
        glClear(GL_DEPTH_BUFFER_BIT); // Only clear depth buffer
        glUseProgram(zPassProgram);
        glBindVertexArray(vao);
        // Render our floor
        glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
        glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getView()));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. LIGHTS
        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Do color writing
        glDepthMask(GL_FALSE); // Do not write depth anymore
        glClear(GL_COLOR_BUFFER_BIT); // Only clear color buffer
        glDepthFunc(GL_EQUAL);
        glUseProgram(lightPassProgram);
        glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
        glUniformMatrix4fv(glGetUniformLocation(lightPassProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getView()));
        for(int i = 0; i < 3; ++i)
        {
            // Render the floor for each light
            // 
            // We optimize this part by only rendering the part of the screen that is affected by a light.
            // We calculate the light's position in screen space and use the stencil buffer to render only that part of the screen.
            lightBBScreen(lights[i], camera.getProjection(), camera.getView(), width, height, boundingBox);
            // Update the stencil buffer
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Comment this line to see the contents of the stencil buffer
            glDisable(GL_DEPTH_TEST);
            glClear(GL_STENCIL_BUFFER_BIT); // Clear the stencil buffer
            glStencilFunc(GL_ALWAYS, 1, 0xFF); // If a fragment is drawn, set it to 1
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilMask(0xFF); // Write to stencil buffer
            drawQuad(stencilProgram, boundingBox, width, height);
            glStencilMask(0x00); // No longer write to stencil buffer
            glEnable(GL_DEPTH_TEST);
            glStencilFunc(GL_EQUAL, 1, 0xFF); // If a fragment's stencil buffer value is 1, draw it
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Draw to the color buffer again

            // Draw the geometry for the light
            glUseProgram(lightPassProgram);
            glBindVertexArray(vao);
            glUniform3fv(glGetUniformLocation(lightPassProgram, "lightPosition"), 1, glm::value_ptr(lights[i].position));
            glUniform3fv(glGetUniformLocation(lightPassProgram, "lightColor"), 1, glm::value_ptr(lights[i].color));
            glUniform3fv(glGetUniformLocation(lightPassProgram, "lightAtt"), 1, glm::value_ptr(lights[i].attenuation));
            // Additionally, we only render objects within the light radius (here: all objects = floor).
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glDisable(GL_STENCIL_TEST);

        glUseProgram(0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(zPassProgram);

    glfwTerminate();
    return 0;
}

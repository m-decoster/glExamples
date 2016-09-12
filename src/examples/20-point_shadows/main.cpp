#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include "util.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// See ./shader.cpp
extern const char* VERTEX_Z_PASS_SRC;
extern const char* FRAGMENT_Z_PASS_SRC;
extern const char* VERTEX_LIGHT_SRC;
extern const char* FRAGMENT_LIGHT_SRC;
extern const char* VERTEX_SHADOW_SRC;
extern const char* GEOM_SHADOW_SRC;
extern const char* FRAGMENT_SHADOW_SRC;

static const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
static int g_screenWidth, g_screenHeight;
static bool shouldUpdateShadowMaps;

void depthPrePass(GLuint zPassProgram, GLuint vao, const std::vector<glm::mat4>& cubeModels, const glm::mat4& floorModel, Camera& camera)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Don't write colors
    glDepthMask(GL_TRUE); // Do depth writing
    glDepthFunc(GL_LESS);
    glClear(GL_DEPTH_BUFFER_BIT); // Only clear depth buffer
    glUseProgram(zPassProgram);
    glBindVertexArray(vao);

    glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(floorModel));
    glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
    glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getView()));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    for (auto it = cubeModels.begin(); it != cubeModels.end(); ++it)
    {
        glUniformMatrix4fv(glGetUniformLocation(zPassProgram, "model"), 1, GL_FALSE, glm::value_ptr(*it));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void shadowPass(GLuint shadowProgram, GLuint depthCubemap, GLuint depthMapFBO, const glm::mat4& shadowProj, const glm::vec3& lightPos, const std::vector<glm::mat4>& cubeModels, const glm::mat4& floorModel, float far, GLuint vao)
{
    glDepthMask(GL_TRUE); // Do depth writing
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(vao);

    // View matrix for each face
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.reserve(6);
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,0.0,1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,-1.0,0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,-1.0,0.0)));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadowProgram);

    for(int i = 0; i < 6; ++i)
    {
        std::string name = "shadowMatrices[";
        name += std::to_string(i) + "]";
        glUniformMatrix4fv(glGetUniformLocation(shadowProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
    }
    glUniform1f(glGetUniformLocation(shadowProgram, "far_plane"), far);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    glUniform3fv(glGetUniformLocation(shadowProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    
    glUniformMatrix4fv(glGetUniformLocation(shadowProgram, "model"), 1, GL_FALSE, glm::value_ptr(floorModel));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    for (auto it = cubeModels.begin(); it != cubeModels.end(); ++it)
    {
        glUniformMatrix4fv(glGetUniformLocation(shadowProgram, "model"), 1, GL_FALSE, glm::value_ptr(*it));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, g_screenWidth, g_screenHeight);
    glEnable(GL_BLEND);
}

static GLint lightPositionLocation = -1,
             lightColorLocation = -1,
             lightAttLocation = -1,
             viewPosLocation = -1,
             projectionLocation = -1,
             viewLocation = -1,
             modelLocation = -1,
             far_planeLocation = -1,
             depthMapLocation = -1;

void lightPass(GLuint lightPassProgram, GLuint vao, const std::vector<glm::mat4>& cubeModels, const glm::mat4& floorModel, Camera& camera, int width, int height, int* boundingBox, PointLight& light, float far)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Do color writing
    glDepthMask(GL_FALSE); // Do not write depth anymore
    glClear(GL_COLOR_BUFFER_BIT); // Only clear color buffer
    glDepthFunc(GL_EQUAL);

    glBindVertexArray(vao);
    glUseProgram(lightPassProgram);
    if(lightPositionLocation == -1)
    {
#define GLUL(s) glGetUniformLocation(lightPassProgram, s)
        lightPositionLocation = GLUL("lightPosition");
        lightColorLocation = GLUL("lightColor");
        lightAttLocation = GLUL("lightAtt");
        viewPosLocation = GLUL("viewPos");
        projectionLocation = GLUL("projection");
        viewLocation = GLUL("view");
        modelLocation = GLUL("model");
        far_planeLocation = GLUL("far_plane");
        depthMapLocation = GLUL("depthMap");
    }
    glUniform3fv(lightPositionLocation, 1, glm::value_ptr(light.position));
    glUniform3fv(lightColorLocation, 1, glm::value_ptr(light.color));
    glUniform3fv(lightAttLocation, 1, glm::value_ptr(light.attenuation));
    glUniform3fv(viewPosLocation, 1, glm::value_ptr(camera.getPosition()));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera.getProjection()));
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.getView()));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(floorModel));
    glUniform1f(far_planeLocation, far);
    glUniform1i(depthMapLocation, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    for (auto it = cubeModels.begin(); it != cubeModels.end(); ++it)
    {
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(*it));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glDisable(GL_STENCIL_TEST);
}

int main(void)
{
    GLFWwindow* window;
    window = init("Point Shadows", 640, 480);
    if(!window)
    {
        return -1;
    }

    glfwGetFramebufferSize(window, &g_screenWidth, &g_screenHeight);

    glm::mat4 floorModel = glm::scale(glm::mat4(), glm::vec3(100.0f, 1.0f, 100.0f));
    glm::mat4 cubeModel0 = glm::translate(glm::mat4(), glm::vec3(-5.0f, 1.0f, -5.0f));
    glm::mat4 cubeModel1 = glm::translate(glm::mat4(), glm::vec3(5.0f, 1.0f, 0.0f));
    glm::mat4 cubeModel2 = glm::translate(glm::mat4(), glm::vec3(-5.0f, 1.0f, 5.0f));
    std::vector<glm::mat4> cubeModels = { cubeModel0, cubeModel1, cubeModel2 };

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    camera.setPosition(0.0f, 1.0f, 0.0f);
    setCamera(&camera);

    GLuint zPassProgram, lightPassProgram, shadowProgram;
    { // zPassProgram
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
    { // lightPassProgram
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
    { // shadowProgram
        GLuint vertex = createShader(VERTEX_SHADOW_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_SHADOW_SRC, GL_FRAGMENT_SHADER);
        GLuint geometry = createShader(GEOM_SHADOW_SRC, GL_GEOMETRY_SHADER);
        shadowProgram = createShaderProgram(vertex, geometry, fragment);
        linkShader(shadowProgram);
        validateShader(shadowProgram);
        glDetachShader(shadowProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(shadowProgram, geometry);
        glDeleteShader(geometry);
        glDetachShader(shadowProgram, fragment);
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

    PointLight light(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.75f, 0.75f, 0.75f), glm::vec3(1.0f, 0.35, 0.44));

    // Generate depth cubemap
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    GLuint depthCubemap;
    glGenTextures(1, &depthCubemap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);  
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // Attach as depth component
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE); // Doesn't render to color buffer
    glReadBuffer(GL_NONE); // Doesn't render to color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    // Shadow projection matrix
    GLfloat aspect = (GLfloat)SHADOW_WIDTH/(GLfloat)SHADOW_HEIGHT;
    GLfloat near = 0.001f;
    GLfloat far = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far); // 90.0f for face alignment

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    // Additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    shouldUpdateShadowMaps = true;
    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }
        if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        {
            light.position.y += 0.1;
            shouldUpdateShadowMaps = true;
        }
        if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        {
            light.position.y -= 0.1;
            shouldUpdateShadowMaps = true;
        }
        if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        {
            light.position.x -= 0.1;
            shouldUpdateShadowMaps = true;
        }
        if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            light.position.x += 0.1;
            shouldUpdateShadowMaps = true;
        }

        updateCamera(640, 480, window);

        int boundingBox[4];

        if(shouldUpdateShadowMaps)
        {
            shadowPass(shadowProgram, depthCubemap, depthMapFBO, shadowProj, light.position, cubeModels, floorModel, far, vao);
        }
        depthPrePass(zPassProgram, vao, cubeModels, floorModel, camera);
        lightPass(lightPassProgram, vao, cubeModels, floorModel, camera, g_screenWidth, g_screenHeight, boundingBox, light, far);

        glfwSwapBuffers(window);
        glfwPollEvents();

        shouldUpdateShadowMaps = false;
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(zPassProgram);
    glDeleteProgram(lightPassProgram);
    glDeleteProgram(shadowProgram);
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthCubemap);

    glfwTerminate();
    return 0;
}

#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define SHADOW_W 1024
#define SHADOW_H SHADOW_W

const char* VERTEX_SP_SRC = "#version 330 core\n"
                            "layout(location=0) in vec3 position;"
                            "uniform mat4 lightSpace;"
                            "uniform mat4 model;"
                            "void main()"
                            "{"
                            "    gl_Position = lightSpace * model * vec4(position, 1.0);"
                            "}";

const char* FRAGMENT_SP_SRC = "#version 330 core\n"
                              "void main()"
                              "{"
                              "    gl_FragDepth = gl_FragCoord.z;"
                              "}";

const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec3 position;"
                         "layout(location=1) in vec3 normal;"
                         "layout(location=2) in vec2 texCoords;"
                         "out vec3 fNormal;"
                         "out vec3 fPosition;"
                         "out vec2 fTexCoords;"
                         "out vec4 fLightSpacePos;" // NEW
                         "uniform mat4 model;"
                         "uniform mat4 lightSpace;" // NEW: the light space matrix
                         "layout(std140) uniform PV"
                         "{"
                         "    mat4 projection;"
                         "    mat4 view;"
                         "};"
                         "void main()"
                         "{"
                         "    gl_Position = projection * view * model * vec4(position, 1.0);"
                         "    fPosition = (model * vec4(position, 1.0)).xyz;" // The position of the vertex in world space
                         "    fNormal = mat3(transpose(inverse(model))) * normal;" // Calculate the normal matrix to have correct normals after scaling
                         "    fTexCoords = texCoords;"
                         "    fLightSpacePos = lightSpace * vec4(fPosition, 1.0);" // NEW
                         "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "#define NUM_POINT_LIGHTS 3\n"
                           "struct Sun"
                           "{"
                           "    vec3 dir;"
                           "    vec3 ambient;"
                           "    vec3 diffuse;"
                           "    vec3 specular;"
                           "};"
                           "struct PointLight"
                           "{"
                           "    vec3 position;"
                           "    vec3 att;" // x = constant, y = linear, z = quadratic
                           "    vec3 ambient;"
                           "    vec3 diffuse;"
                           "    vec3 specular;"
                           "};"
                           "in vec3 fNormal;"
                           "in vec3 fPosition;"
                           "in vec2 fTexCoords;"
                           "in vec4 fLightSpacePos;"
                           "out vec4 outputColor;"
                           "layout(std140) uniform Light"
                           "{"
                           "    Sun sun;"
                           "    PointLight pointLights[NUM_POINT_LIGHTS];"
                           "};"
                           "uniform vec3 eye;"
                           "uniform sampler2D matDiffuse;"
                           "uniform sampler2D matSpecular;"
                           "uniform sampler2D shadowMap;"
                           "uniform float matShine;"
                           "vec3 dirLight(Sun sun, vec3 normal, vec3 eye, float shdw)" // NEW
                           "{"
                           "    vec3 dir = normalize(-sun.dir);"
                           "    float diff = max(dot(normal, dir), 0.0);"
                           "    vec3 hwd = normalize(dir + eye);"
                           "    float spec = pow(max(dot(normal, hwd), 0.0), matShine);"
                           "    vec3 ambient = sun.ambient * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 diffuse = sun.diffuse * diff * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 specular = sun.specular * spec * vec3(texture(matSpecular, fTexCoords));"
                           "    return ambient + (1.0 - shdw) * (diffuse + specular);" // NEW
                           "}"
                           "vec3 pointLight(PointLight light, vec3 normal, vec3 pos, vec3 eye, float shdw)" // NEW
                           "{"
                           "    vec3 dir = normalize(light.position - pos);"
                           "    float diff = max(dot(normal, dir), 0.0);"
                           "    vec3 hwd = normalize(dir + eye);"
                           "    float spec = pow(max(dot(normal, hwd), 0.0), matShine);"
                           "    float dist = length(light.position - pos);"
                           "    float attenuation = 1.0f / (light.att.x + light.att.y * dist + light.att.z * dist * dist);"
                           "    vec3 ambient = light.ambient * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 diffuse = light.diffuse * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 specular = light.specular * spec * vec3(texture(matSpecular, fTexCoords));"
                           "    return (ambient + (1.0 - shdw) * (diffuse + specular)) * attenuation;" // NEW
                           "}"
                           "float shadow(vec4 pos)" // NEW function
                           "{"
                           "    vec3 projCoords = pos.xyz / pos.w;"
                           "    projCoords = projCoords * 0.5 + 0.5;" // From [-1,1] to [0,1]
                           "    float depth = texture(shadowMap, projCoords.xy).r;"
                           "    float curDepth = projCoords.z;"
                           "    float shadow = curDepth > depth ? 1.0 : 0.0;"
                           "    return shadow;"
                           "}"
                           "void main()"
                           "{"
                           "    float shdw = shadow(fLightSpacePos);" // NEW
                           "    vec3 normal = normalize(fNormal);"
                           "    vec3 eyeDir = normalize(eye - fPosition);"
                           "    vec3 result = dirLight(sun, normal, eyeDir, shdw);" // NEW
                           "    for(int i = 0; i < NUM_POINT_LIGHTS; ++i)"
                           "    {"
                           "        result += pointLight(pointLights[i], normal, fPosition, eyeDir, shdw);" // NEW
                           "    }"
                           "    outputColor = vec4(result, 1.0);"
                           "}";

GLuint program, shadowProgram, vao, depthMap, depthMapFBO;
glm::mat4 model, floorModel, lightSpace;
Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
glm::vec3 lightPos(5.0f, 5.0f, 5.0f);

void shadowPass()
{
    // Render the scene from the position of the directional light
    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 lightProj, lightView;
    lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    lightSpace = lightProj * lightView;

    glBindVertexArray(vao);
    glUseProgram(shadowProgram);
    glUniformMatrix4fv(glGetUniformLocation(shadowProgram, "lightSpace"), 1, GL_FALSE, glm::value_ptr(lightSpace));
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(floorModel));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void geomPass()
{
    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniformMatrix4fv(glGetUniformLocation(shadowProgram, "lightSpace"), 1, GL_FALSE, glm::value_ptr(lightSpace));
    glUniform3fv(glGetUniformLocation(program, "eye"), 1, glm::value_ptr(camera.getPosition()));
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(floorModel));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glUseProgram(0);
}

int main(void)
{
    GLFWwindow* window;
    window = init("Shadows", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB); // Gamma correction

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    setCamera(&camera);

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
    {
        GLuint vertex = createShader(VERTEX_SP_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_SP_SRC, GL_FRAGMENT_SHADER);
        shadowProgram = createShaderProgram(vertex, fragment);
        linkShader(shadowProgram);
        validateShader(shadowProgram);
        glDetachShader(shadowProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(shadowProgram, fragment);
        glDeleteShader(fragment);
    }

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

    // Shadow map (depth map) fbo
    glGenFramebuffers(1, &depthMapFBO);  
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // Don't write to the color buffer
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Projection/View matrix ubo
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);

    GLuint pv_index = glGetUniformBlockIndex(program, "PV");   
    glUniformBlockBinding(program, pv_index, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Light ubo
    GLuint lightUbo;
    glGenBuffers(1, &lightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, lightUbo);
    // We have static lights in this example so we can fill the uniform buffer once and be done with it
    float bufferData[] =
    {
        // sunlight
        /* dir */ -0.5f, -0.5f, -0.5f, 0.0f, /* ambient */ 0.2f, 0.2f, 0.2f, 0.0f, /* diffuse */ 0.9f, 0.9f, 0.9f, 0.0f, /* specular */ 0.9f, 0.9f, 0.9f, 0.0f,
        // pointlight 0
        /* pos */ 0.0f, 5.0f, 0.0f, 0.0f, /* attenuation */ 1.0f, 0.7f, 1.8f, 0.0f, /* ambient */ 0.3f, 0.1f, 0.1f, 0.0f, /* diffuse */ 0.5f, 1.0f, 0.5f, 0.0f, /* specular */ 0.5f, 0.5f, 0.5f, 0.0f,
        // pointlight 1
        /* pos */ 5.0f, 5.0f, 0.0f, 0.0f, /* attenuation */ 1.0f, 0.35f, 0.44f, 0.0f, /* ambient */ 0.3f, 0.1f, 0.1f, 0.0f, /* diffuse */ 0.7f, 0.5f, 0.2f, 0.0f, /* specular */ 0.5f, 0.5f, 0.5f, 0.0f,
        // pointlight 2
        /* pos */ 0.0f, -5.0f, 5.0f, 0.0f, /* attenuation */ 1.0f, 0.35f, 0.44f, 0.0f, /* ambient */ 0.3f, 0.1f, 0.1f, 0.0f, /* diffuse */ 0.7f, 0.5f, 0.2f, 0.0f,/* specular */ 0.5f, 0.5f, 0.5f, 0.0f
    }; // Vec3: we add a padding float after each vec3
    glBufferData(GL_UNIFORM_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    GLuint light_index = glGetUniformBlockIndex(program, "Light");
    glUniformBlockBinding(program, light_index, 1);

    int w, h;
    GLuint diffuse = loadImage("diffuseCube.png", &w, &h, 0, false);
    if(!diffuse)
    {
        return -1;
    }
    GLuint specular = loadImage("specularCube.png", &w, &h, 1, false);
    if(!specular)
    {
        return -1;
    }

    // The second cube is larger
    floorModel = glm::scale(floorModel, glm::vec3(100.0f, 1.0f, 100.0f));
    floorModel = glm::translate(floorModel, glm::vec3(0.0f, -5.0f, 0.0f));

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "matDiffuse"), 0);
    glUniform1i(glGetUniformLocation(program, "matSpecular"), 1);
    glUniform1f(glGetUniformLocation(program, "matShine"), 32.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Fill the Projection matrix in the P/V ubo
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera.getProjection()));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        updateCamera(640, 480, window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera.getView()));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        shadowPass();
        geomPass();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ubo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &diffuse);
    glDeleteTextures(1, &specular);
    glDeleteProgram(program);
    glDeleteProgram(shadowProgram);

    glfwTerminate();
    return 0;
}

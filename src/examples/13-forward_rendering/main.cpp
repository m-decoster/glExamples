#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec3 position;"
                         "layout(location=1) in vec3 normal;"
                         "layout(location=2) in vec2 texCoords;"
                         "out vec3 fNormal;"
                         "out vec3 fPosition;"
                         "out vec2 fTexCoords;"
                         "uniform mat4 model;"
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
                           "out vec4 outputColor;"
                           "layout(std140) uniform Light"
                           "{"
                           "    Sun sun;"
                           "    PointLight pointLights[NUM_POINT_LIGHTS];"
                           "};"
                           "uniform vec3 eye;"
                           "uniform sampler2D matDiffuse;"
                           "uniform sampler2D matSpecular;"
                           "float matShine;"
                           "vec3 dirLight(Sun sun, vec3 normal, vec3 eye)"
                           "{"
                           "    vec3 dir = normalize(-sun.dir);"
                           "    float diff = max(dot(normal, dir), 0.0);"
                           "    float spec = pow(max(dot(eye, reflect(-dir, normal)), 0.0), matShine);"
                           "    vec3 ambient = sun.ambient * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 diffuse = sun.diffuse * diff * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 specular = sun.specular * spec * vec3(texture(matSpecular, fTexCoords));"
                           "    return ambient + diffuse + specular;"
                           "}"
                           "vec3 pointLight(PointLight light, vec3 normal, vec3 eye, vec3 pos)"
                           "{"
                           "    vec3 dir = normalize(light.position - pos);"
                           "    float diff = max(dot(normal, dir), 0.0);"
                           "    float spec = pow(max(dot(eye, reflect(-dir, normal)), 0.0), matShine);"
                           "    float dist = length(light.position - pos);"
                           "    float attenuation = 1.0f / (light.att.x + light.att.y * dist + light.att.z * dist * dist);"
                           "    vec3 ambient = light.ambient * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 diffuse = light.diffuse * vec3(texture(matDiffuse, fTexCoords));"
                           "    vec3 specular = light.specular * spec * vec3(texture(matSpecular, fTexCoords));"
                           "    return (ambient + diffuse + specular) * attenuation;"
                           "}"
                           "void main()"
                           "{"
                           "    vec3 normal = normalize(fNormal);"
                           "    vec3 eyeDir = normalize(eye - fPosition);"
                           "    vec3 result = dirLight(sun, normal, eyeDir);"
                           "    for(int i = 0; i < NUM_POINT_LIGHTS; ++i)"
                           "    {"
                           "        result += pointLight(pointLights[i], normal, fPosition, eyeDir);"
                           "    }"
                           "    outputColor = vec4(result, 1.0);"
                           "}";

int main(void)
{
    GLFWwindow* window;
    window = init("Forward Rendering", 640, 480);
    if(!window)
    {
        return -1;
    }

    glm::mat4 model;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB); // Gamma correction

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, 640.0f, 480.0f);
    setCamera(&camera);

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
        /* dir */ -0.5f, -0.5f, -0.5f, 0.0f, /* ambient */ 0.2f, 0.2f, 0.2f, 0.0f, /* diffuse */ 0.7f, 0.2f, 0.2f, 0.0f, /* specular */ 0.5f, 0.5f, 0.5f, 0.0f,
        // pointlight 0
        /* pos */ 0.0f, 5.0f, 0.0f, 0.0f, /* attenuation */ 1.0f, 0.7f, 1.8f, 0.0f, /* ambient */ 0.3f, 0.1f, 0.1f, 0.0f, /* diffuse */ 0.3f, 1.0f, 0.2f, 0.0f, /* specular */ 0.8f, 0.6f, 0.3f, 0.0f,
        // pointlight 1
        /* pos */ 5.0f, 0.0f, 0.0f, 0.0f, /* attenuation */ 1.0f, 0.35f, 0.44f, 0.0f, /* ambient */ 0.3f, 0.1f, 0.1f, 0.0f, /* diffuse */ 0.7f, 0.5f, 0.2f, 0.0f, /* specular */ 0.8f, 0.6f, 0.3f, 0.0f,
        // pointlight 2
        /* pos */ 0.0f, -5.0f, 5.0f, 0.0f, /* attenuation */ 1.0f, 0.35f, 0.44f, 0.0f, /* ambient */ 0.3f, 0.1f, 0.1f, 0.0f, /* diffuse */ 0.7f, 0.5f, 0.2f, 0.0f,/* specular */ 0.8f, 0.6f, 0.3f, 0.0f
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

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "matDiffuse"), 0);
    glUniform1i(glGetUniformLocation(program, "matSpecular"), 1);
    glUniform1f(glGetUniformLocation(program, "matShine"), 32.0f);
    glUniform3fv(glGetUniformLocation(program, "eye"), 1, glm::value_ptr(camera.getPosition()));

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

        glBindVertexArray(vao);

        glUseProgram(program);

        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

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

    glfwTerminate();
    return 0;
}

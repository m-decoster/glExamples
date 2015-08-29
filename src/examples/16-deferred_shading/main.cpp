#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WIDTH 640
#define HEIGHT 480
#define NUM_ASTEROIDS 2000
#define NUM_POINT_LIGHTS 32
#define SEED 1993

const char* VERTEX_GEOM_SRC = "#version 330 core\n"
                              "layout(location=0) in vec3 position;"
                              "layout(location=1) in vec3 normal;"
                              "layout(location=2) in vec2 texCoord;"
                              "layout(location=3) in mat4 model_inst;"
                              "uniform mat4 view;"
                              "uniform mat4 projection;"
                              "uniform float specularPower;"
                              "out vec3 fPosition;"
                              "out vec3 fNormal;"
                              "out vec2 fTexCoord;"
                              "out float fSpecularPower;"
                              "void main()"
                              "{"
                              "    vec4 wP = model_inst * vec4(position, 1.0);"
                              "    fPosition = wP.xyz;"
                              "    gl_Position = projection * view * wP;"
                              "    fNormal = transpose(inverse(mat3(model_inst))) * normal;"
                              "    fTexCoord = texCoord;"
                              "    fSpecularPower = specularPower;"
                              "}";

const char* FRAGMENT_GEOM_SRC = "#version 330 core\n"
                                "layout(location=0) out vec3 g_position;"
                                "layout(location=1) out vec4 g_normal_spec_pow;" // rgb: normal, a: specular power
                                "layout(location=2) out vec4 g_albedo_spec;" // rgb: albedo, a: spec
                                "in vec3 fPosition;"
                                "in vec3 fNormal;"
                                "in vec2 fTexCoord;"
                                "in float fSpecularPower;"
                                "uniform sampler2D diffuse;"
                                "uniform sampler2D specular;"
                                "void main()"
                                "{"
                                "    g_position = fPosition;"
                                "    g_normal_spec_pow.rgb = normalize(fNormal);"
                                "    g_normal_spec_pow.a = fSpecularPower;"
                                "    g_albedo_spec = texture(diffuse, fTexCoord);"
                                "    g_albedo_spec.a = texture(specular, fTexCoord).r;"
                                "}";

const char* VERTEX_LIGHT_SRC = "#version 330 core\n"
                               "layout(location=0) in vec2 position;"
                               "layout(location=1) in vec2 texCoord;"
                               "out vec2 fTexCoord;"
                               "void main()"
                               "{"
                               "    gl_Position = vec4(position, 0.0, 1.0);"
                               "    fTexCoord = texCoord;"
                               "}";

const char* FRAGMENT_LIGHT_SRC = "#version 330 core\n"
                                 "#define NUM_POINT_LIGHTS 32\n"
                                 "struct PointLight"
                                 "{"
                                 "    vec3 position;"
                                 "    vec3 att;" // x = constant, y = linear, z = quadratic
                                 "    vec3 ambient;"
                                 "    vec3 diffuse;"
                                 "    vec3 specular;"
                                 "};"
                                 "in vec2 fTexCoord;"
                                 "uniform PointLight lights[NUM_POINT_LIGHTS];"
                                 "uniform vec3 eye;"
                                 "uniform sampler2D g_position;"
                                 "uniform sampler2D g_normal_spec_pow;"
                                 "uniform sampler2D g_albedo_spec;"
                                 "out vec4 outputColor;"
                                 "vec3 pointLight(PointLight light, vec3 normal, vec3 pos, vec3 eye, vec3 albedo, float pSpecular, float specularPower)"
                                 "{"
                                 "    vec3 dir = normalize(light.position - pos);"
                                 "    vec3 ambient = albedo * light.ambient;"
                                 "    vec3 diffuse = max(dot(normal, dir), 0.0) * albedo * light.diffuse;"
                                 "    vec3 hwd = normalize(dir + eye);"
                                 "    float spec = pow(max(dot(normal, hwd), 0.0), specularPower);"
                                 "    vec3 specular = light.specular * spec * pSpecular;"
                                 "    float dist = length(light.position - pos);"
                                 "    float attenuation = 1.0f / (light.att.x + light.att.y * dist + light.att.z * dist * dist);"
                                 "    return (ambient + diffuse + specular) * attenuation;"
                                 "}"
                                 "void main()"
                                 "{"
                                 "    vec3 fPosition = texture(g_position, fTexCoord).rgb;"
                                 "    vec3 fNormal = texture(g_normal_spec_pow, fTexCoord).rgb;"
                                 "    float specularPower = texture(g_normal_spec_pow, fTexCoord).a;"
                                 "    vec3 albedo = texture(g_albedo_spec, fTexCoord).rgb;"
                                 "    float specular = texture(g_albedo_spec, fTexCoord).a;"
                                 "    vec3 eyeDir = normalize(eye - fPosition);"
                                 "    vec3 result;"
                                 "    for(int i = 0; i < NUM_POINT_LIGHTS; ++i)"
                                 "    {"
                                 "       result += pointLight(lights[i], fNormal, fPosition, eyeDir, albedo, specular, specularPower);"
                                 "    }"
                                 "    outputColor = vec4(result, 1.0);"
                                 "}";


/*
 * Multiple render targets!
 * We create a frame buffer with several GL_COLOR_ATTACHMENTs,
 * which allows us to render to multiple textures at once
 */
struct GBuffer
{
    GBuffer()
    {
        // Generate a framebuffer
        glGenFramebuffers(1, &buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);

        // Create a texture for the position buffer
        glGenTextures(1, &position);
        glBindTexture(GL_TEXTURE_2D, position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Bind the position buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position, 0);

        // Create a texture for the normal buffer
        glGenTextures(1, &normal);
        glBindTexture(GL_TEXTURE_2D, normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Bind the normal buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0);

        // Create a texture for the color buffer
        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Bind the color buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, color, 0);

        GLuint colorAttachments[] =
        {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };

        glDrawBuffers(3, colorAttachments);

        // Create a depth buffer
        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
        // Bind the depth buffer to the framebuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Could not create G-Buffer!" << std::endl
                << "Error number: " << glGetError() << std::endl;
            exit(1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

    ~GBuffer()
    {
        // or glDeleteTextures(3, &position);
        glDeleteTextures(1, &position);
        glDeleteTextures(1, &normal);
        glDeleteTextures(1, &color);
        glDeleteRenderbuffers(1, &depth);
        glDeleteFramebuffers(1, &buffer);
    };

    GLuint buffer, position, normal, color, depth;
};

struct PointLight
{
    void set(const glm::vec3& position, const glm::vec3& att, const glm::vec3& ambient,
            const glm::vec3& diffuse, const glm::vec3& specular)
    {
        this->position = position;
        this->att = att;
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
    };

    void setUniforms(int index, GLuint program)
    {
        std::string array = "lights[" + std::to_string(index) + "].";
        glUniform3fv(glGetUniformLocation(program, (array + "position").c_str()), 1, glm::value_ptr(position));
        glUniform3fv(glGetUniformLocation(program, (array + "att").c_str()), 1, glm::value_ptr(att));
        glUniform3fv(glGetUniformLocation(program, (array + "ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(program, (array + "diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(program, (array + "specular").c_str()), 1, glm::value_ptr(specular));
    };

private:
    glm::vec3 position;
    glm::vec3 att;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

int main(void)
{
    GLFWwindow* window;
    window = init("Deferred Shading", WIDTH, HEIGHT);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, (float)WIDTH, (float)HEIGHT);
    setCamera(&camera);

    GLuint geomProgram, lightProgram;
    {
        // Geometry pass program
        GLuint vertex = createShader(VERTEX_GEOM_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_GEOM_SRC, GL_FRAGMENT_SHADER);
        geomProgram = createShaderProgram(vertex, fragment);
        linkShader(geomProgram);
        validateShader(geomProgram);
        glDetachShader(geomProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(geomProgram, fragment);
        glDeleteShader(fragment);
    }
    {
        // Light pass program
        GLuint vertex = createShader(VERTEX_LIGHT_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_LIGHT_SRC, GL_FRAGMENT_SHADER);
        lightProgram = createShaderProgram(vertex, fragment);
        linkShader(lightProgram);
        validateShader(lightProgram);
        glDetachShader(lightProgram, vertex);
        glDeleteShader(vertex);
        glDetachShader(lightProgram, fragment);
        glDeleteShader(fragment);
    }

    // Create quad VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    float vertices[] =
    {
        // x   y       u     v
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Load the diffuse and specular texture
    // TODO: seperate specular texture
    glUseProgram(geomProgram);
    int w, h;
    GLuint textureDiff = loadImage("asteroid.png", &w, &h, 0, false); // GL_TEXTURE0
    if(!textureDiff)
    {
        return -1;
    }
    glUniform1i(glGetUniformLocation(geomProgram, "diffuse"), 0); // GL_TEXTURE0
    GLuint textureSpec = loadImage("asteroid.png", &w, &h, 1, false); // GL_TEXTURE1
    if(!textureSpec)
    {
        return -1;
    }
    glUniform1i(glGetUniformLocation(geomProgram, "specular"), 1); // GL_TEXTURE1

    Mesh mesh;
    if(!mesh.load("asteroid.obj"))
    {
        std::cerr << "Could not load mesh" << std::endl;
        return -1;
    }

    // Set random positions for the asteroids
    std::vector<glm::mat4> models;
    models.resize(NUM_ASTEROIDS);
    srand(SEED);
    for(int i = 0; i < NUM_ASTEROIDS; ++i)
    {
        glm::mat4 model;

        // Translate
        model = glm::translate(model, glm::vec3(rand() % 100 - 50.0f, rand() % 100 - 50.0f, rand() % 100));

        // Scale
        float scale = (rand() % 200) / 100.0f + 0.1f;
        model = glm::scale(model, glm::vec3(scale, scale, scale));

        // Rotate
        model = glm::rotate(model, glm::radians((float)(rand() % 100)), glm::vec3(1.0f, 1.0f, 1.0f));

        models[i] = model;
    }

    mesh.setInstances(NUM_ASTEROIDS, models);

    // Set positions and more for the lights
    PointLight lights[NUM_POINT_LIGHTS];
    for(int i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        float x, y, z;
        x = rand() % 100 - 50.0f;
        y = rand() % 100 - 50.0f;
        z = rand() % 100 - 50.0f;

        float r, g, b; // For simplicity's sake, ambient, diffuse and specular colours are the same
        r = ((rand() % 100) / 200.0f) + 0.5;
        g = ((rand() % 100) / 200.0f) + 0.5;
        b = ((rand() % 100) / 200.0f) + 0.5;

        float con, lin, qua;
        con = 1.0f;
        lin = 0.5f;
        qua = 1.5f;

        lights[i].set(glm::vec3(x, y, z), glm::vec3(con, lin, qua), glm::vec3(r, g, b),
                glm::vec3(r, g, b), glm::vec3(r, g, b));
    }

    GBuffer gBuffer;

    glUseProgram(lightProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.position);
    glUniform1i(glGetUniformLocation(lightProgram, "g_position"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
    glUniform1i(glGetUniformLocation(lightProgram, "g_normal_spec_pow"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.color);
    glUniform1i(glGetUniformLocation(lightProgram, "g_albedo_spec"), 2);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        updateCamera(WIDTH, HEIGHT, window);

        // GEOMETRY PASS
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 proj = camera.getProjection();
        glm::mat4 view = camera.getView();
        glUseProgram(geomProgram);
        glUniformMatrix4fv(glGetUniformLocation(geomProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(geomProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(glGetUniformLocation(geomProgram, "specularPower"), 16.0f);
        mesh.render();
        
        // LIGHT PASS
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(lightProgram); 
        glUniform3fv(glGetUniformLocation(lightProgram, "eye"), 1, glm::value_ptr(camera.getPosition()));
        for(int i = 0; i < NUM_POINT_LIGHTS; ++i)
        {
            lights[i].setUniforms(i, lightProgram);
        }
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteTextures(1, &textureDiff);
    glDeleteTextures(1, &textureSpec);
    glDeleteProgram(geomProgram);
    glDeleteProgram(lightProgram);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwTerminate();
    return 0;
}

#include "../common/util.h"
#include "../common/shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#define CUR_POS_VBO 0
#define NEXT_POS_VBO 1
#define CUR_TBO 2
#define NEXT_TBO 3
#define EBO 4

const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec3 curPos;" // current frame position of vertices
                         "layout(location=1) in vec3 nextPos;" // next frame position of vertices
                         "layout(location=2) in vec2 curTC;" // current frame texcoords
                         "layout(location=3) in vec2 nextTC;" // next frame texcoords
                         "uniform mat4 model;"
                         "uniform mat4 view;"
                         "uniform mat4 projection;"
                         "uniform float tween;" // used for linear interpolation
                         "out vec2 fTexcoord;"
                         "void main()"
                         "{"
                         "    fTexcoord = mix(curTC, nextTC, tween);" // linear interpolation
                         "    gl_Position = vec4(mix(curPos, nextPos, tween), 1.0);" // linear interpolation
                         "    gl_Position = projection * view * model * gl_Position;"
                         "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec2 fTexcoord;"
                           "uniform sampler2D diffuse;"
                           "out vec4 outputColor;"
                           "void main()"
                           "{"
                           "    outputColor = texture(diffuse, fTexcoord);"
                           "}";

void loadMesh(const char* fileName, std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices, std::vector<glm::vec2>& texCoords, int& numIndices)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fileName,
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);
    if(!scene)
    {
        std::cerr << "Error loading mesh " << fileName << ": " << importer.GetErrorString() << std::endl;
        return;
    }
    const aiMesh* mesh = scene->mMeshes[0];
    for(int  i = 0; i < mesh->mNumVertices; ++i)
    {
        const aiVector3D* pos = &(mesh->mVertices[i]);
        const aiVector3D* texCoord = &(mesh->mTextureCoords[0][i]);
        
        vertices.push_back(glm::vec3(pos->x, pos->y, pos->z));
        texCoords.push_back(glm::vec2(texCoord->x, texCoord->y));
    }
    for(int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace* face = &(mesh->mFaces[i]);
        indices.push_back(face->mIndices[0]);
        indices.push_back(face->mIndices[1]);
        indices.push_back(face->mIndices[2]);
    }
    numIndices = indices.size();
}

int main(void)
{
    GLFWwindow* window = init("Morph Target Animation", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -4.0f));
    glm::mat4 model;

    // Load the shader
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

        if(!program)
        {
            return -1;
        }
    }
    glUseProgram(program);

    // We create the animation from several obj files
    std::vector<glm::vec3> frames[3];
    std::vector<glm::vec2> texCoords[3];
    std::vector<GLuint> indices, _;
    int numIndices, _2;
    loadMesh("frame1.obj", frames[0], indices, texCoords[0], numIndices);
    loadMesh("frame2.obj", frames[1], _, texCoords[1], _2); _.clear();
    loadMesh("frame3.obj", frames[2], _, texCoords[2], _2);
    float times[] =
    {
        1.0f, 1.0f, 1.0f
    }; // The time in seconds between different frames
    // times[0] indicates the time between the first and second frame, and so on
    int numFrames = 3;

    // Create the vao and buffer objects
    GLuint vao;
    GLuint buffers[5];
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(5, buffers);

    // A better way to handle this would be to interleave positions and texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, buffers[CUR_POS_VBO]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * frames[0].size(), &frames[0][0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[NEXT_POS_VBO]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * frames[1].size(), &frames[1][0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[CUR_TBO]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texCoords[0].size(), &texCoords[0][0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[NEXT_TBO]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texCoords[1].size(), &texCoords[1][0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[EBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Load the texture
    int w, h;
    GLuint texture = loadImage("mta_texture.png", &w, &h, 0, false);
    if(!texture)
    {
        return -1;
    }
    glUniform1i(glGetUniformLocation(program, "diffuse"), 0);

    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    float timeSinceFrame = 0.0f;
    int currentFrame;
    glBindVertexArray(vao);
    while(!glfwWindowShouldClose(window))
    {
        // If the time for a frame has passed, we have to switch frames
        timeSinceFrame += glfwGetTime();
        if(timeSinceFrame >= times[currentFrame])
        {
            currentFrame = (currentFrame + 1) % numFrames;
            int nextFrame = (currentFrame + 1) % numFrames;
            timeSinceFrame = 0.0f;

            // Update the buffers
            glBindBuffer(GL_ARRAY_BUFFER, buffers[CUR_POS_VBO]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * frames[currentFrame].size(), 
                    &frames[currentFrame][0], GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, buffers[NEXT_POS_VBO]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * frames[nextFrame].size(),
                    &frames[nextFrame][0], GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, buffers[CUR_TBO]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texCoords[currentFrame].size(),
                    &texCoords[currentFrame][0], GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, buffers[NEXT_TBO]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texCoords[nextFrame].size(),
                    &texCoords[nextFrame][0], GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1f(glGetUniformLocation(program, "tween"), timeSinceFrame / times[currentFrame]);

        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

        glfwSetTime(0.0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(5, buffers);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}

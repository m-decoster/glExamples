#include "mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

Mesh::Mesh()
    : scale(1.0f, 1.0f, 1.0f)
{
}

Mesh::~Mesh()
{
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
    }
}

bool Mesh::load(const char* fileName)
{
    if(vao)
    {
        // Already loaded
        return false;
    }

    Assimp::Importer importer; 
    const aiScene* scene = importer.ReadFile(fileName,
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);
    if(!scene)
    {
        std::cerr << "Error loading mesh " << fileName << ": " << importer.GetErrorString() << std::endl;
        return false;
    }

    // We only load the first mesh from the Assimp scene here
    std::vector<float> vertices;
    std::vector<GLuint> indices;
    const aiMesh* mesh = scene->mMeshes[0];
    for(int  i = 0; i < mesh->mNumVertices; ++i)
    {
        const aiVector3D* pos = &(mesh->mVertices[i]);
        const aiVector3D* texCoord = &(mesh->mTextureCoords[0][i]);
        const aiVector3D* normal = &(mesh->mNormals[i]);
        
        vertices.push_back(pos->x);
        vertices.push_back(pos->y);
        vertices.push_back(pos->z);
        vertices.push_back(normal->x);
        vertices.push_back(normal->y);
        vertices.push_back(normal->z);
        vertices.push_back(texCoord->x);
        vertices.push_back(texCoord->y);
    }
    for(int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace* face = &(mesh->mFaces[i]);
        indices.push_back(face->mIndices[0]);
        indices.push_back(face->mIndices[1]);
        indices.push_back(face->mIndices[2]);
    }
    numIndices = indices.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2); // texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    return true;
}

void Mesh::setPosition(float x, float y, float z)
{
    position = glm::vec3(x, y, z);
}

void Mesh::setScale(float x, float y, float z)
{
    scale = glm::vec3(x, y, z);
}

void Mesh::setAngle(float x, float y, float z)
{
    angle = glm::vec3(x, y, z);
}

glm::mat4 Mesh::getModelMatrix()
{
    glm::mat4 m;
    m = glm::scale(m, scale);
    m = glm::rotate(m, angle.x, glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::rotate(m, angle.y, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, angle.z, glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::translate(m, position);
    return m;
}

void Mesh::render()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
}

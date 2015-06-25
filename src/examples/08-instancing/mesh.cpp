#include "mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

Mesh::Mesh()
    : numMeshes(1)
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
        
        vertices.push_back(pos->x);
        vertices.push_back(pos->y);
        vertices.push_back(pos->z);
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

    // Upload the vertices to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // Upload the indices to the buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1); // texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // We have now successfully created a drawable Vertex Array Object
    glBindVertexArray(0);
    // We no longer need the buffers
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    return true;
}

void Mesh::setInstances(int numMeshes, std::vector<glm::mat4> models)
{
    this->numMeshes = numMeshes;

    glBindVertexArray(vao);

    // Create a buffer for the model matrices
    GLuint mbo;
    glGenBuffers(1, &mbo);
    glBindBuffer(GL_ARRAY_BUFFER, mbo);
    // Send all the model matrices to the GPU
    glBufferData(GL_ARRAY_BUFFER, numMeshes * sizeof(glm::mat4), &models[0], GL_STATIC_DRAW);
    // If you look at the shader, you will see "layout(locaiton=2) in mat4 model_inst"
    // OpenGL only allows a maximum of vec4 passing through glVertexAttribPointer,
    // so we have to pass 4 vec4s to pass a matrix
    glEnableVertexAttribArray(2); 
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(3); 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(4); 
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(5); 
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    
    // Update the matrix (2, 3, 4, 5) for every 1 instance
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);                                          
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    
    glBindVertexArray(0);
    glDeleteBuffers(1, &mbo);
}

void Mesh::render()
{
    glBindVertexArray(vao);
    glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, numMeshes);
    glBindVertexArray(0);
}

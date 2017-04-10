#include "scene.h"
#include "mesh.h"
#include "material.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Scene::Scene()
{

}

Scene::~Scene()
{
    for(auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
        delete *it;
    }
}

bool Scene::load(const char* fileName)
{
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

    for (int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* aiM = scene->mMeshes[m];

        Mesh* mesh = new Mesh();;
        if (!mesh->load(aiM))
        {
            return false;
        }
        m_meshes.push_back(mesh);
    }

    return true;
}

void Scene::render(Material* mat)
{
    for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
        mat->setUniform("model", (*it)->getModelMatrix());
        (*it)->render();
    }
}

#include "Mesh.h"
#include <iostream>
#include <algorithm>
#include <limits>

bool Mesh::load(const std::string& filename) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, 
        aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | 
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    clear();
    minBB = glm::vec3(std::numeric_limits<float>::max());
    maxBB = glm::vec3(std::numeric_limits<float>::lowest());

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* ai_mesh = scene->mMeshes[i];
        unsigned int baseVertex = vertices.size();

        for (unsigned int j = 0; j < ai_mesh->mNumVertices; ++j) {
            Vertex v;
            v.position = glm::vec3(ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z);
            v.normal = ai_mesh->HasNormals() ? glm::vec3(ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z) : glm::vec3(0.0f);
            vertices.push_back(v);
            minBB = glm::min(minBB, v.position);
            maxBB = glm::max(maxBB, v.position);
        }

        for (unsigned int j = 0; j < ai_mesh->mNumFaces; ++j) {
            aiFace face = ai_mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                indices.push_back(baseVertex + face.mIndices[k]);
            }
        }
    }
    return true;
}

void Mesh::clear() {
    vertices.clear();
    indices.clear();
}

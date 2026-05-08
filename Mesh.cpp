#include "Mesh.h"
#include <iostream>
#include <algorithm>
#include <limits>

bool Mesh::load(const std::string& filename) {
    Assimp::Importer importer;
    // aiProcess_JoinIdenticalVertices can be slow for very large meshes, 
    // but it's often needed for smooth normals.
    const aiScene* scene = importer.ReadFile(filename, 
        aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | 
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    clear();
    
    // Pre-calculate total size to reserve memory
    unsigned int totalVertices = 0;
    unsigned int totalIndices = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        totalVertices += scene->mMeshes[i]->mNumVertices;
        for (unsigned int j = 0; j < scene->mMeshes[i]->mNumFaces; ++j) {
            totalIndices += scene->mMeshes[i]->mFaces[j].mNumIndices;
        }
    }
    vertices.reserve(totalVertices);
    indices.reserve(totalIndices);

    minBB = glm::vec3(std::numeric_limits<float>::max());
    maxBB = glm::vec3(std::numeric_limits<float>::lowest());

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* ai_mesh = scene->mMeshes[i];
        unsigned int baseVertex = static_cast<unsigned int>(vertices.size());

        for (unsigned int j = 0; j < ai_mesh->mNumVertices; ++j) {
            Vertex v;
            const aiVector3D& pos = ai_mesh->mVertices[j];
            v.position = glm::vec3(pos.x, pos.y, pos.z);
            
            if (ai_mesh->HasNormals()) {
                const aiVector3D& norm = ai_mesh->mNormals[j];
                v.normal = glm::vec3(norm.x, norm.y, norm.z);
            } else {
                v.normal = glm::vec3(0.0f);
            }
            
            vertices.push_back(v);
            
            // Inline bounding box update
            if (pos.x < minBB.x) minBB.x = pos.x;
            if (pos.y < minBB.y) minBB.y = pos.y;
            if (pos.z < minBB.z) minBB.z = pos.z;
            if (pos.x > maxBB.x) maxBB.x = pos.x;
            if (pos.y > maxBB.y) maxBB.y = pos.y;
            if (pos.z > maxBB.z) maxBB.z = pos.z;
        }

        for (unsigned int j = 0; j < ai_mesh->mNumFaces; ++j) {
            const aiFace& face = ai_mesh->mFaces[j];
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

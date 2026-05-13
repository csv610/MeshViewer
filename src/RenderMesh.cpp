#include "RenderMesh.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <fstream>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;

std::string RenderMesh::getCachePath(const std::string& filename) {
    try {
        fs::path p = fs::absolute(filename);
        size_t hash = std::hash<std::string>{}(p.string());
        std::string cacheName = "meshviewer_" + std::to_string(hash) + ".cache";
        return (fs::temp_directory_path() / cacheName).string();
    } catch (...) {
        return filename + ".cache"; // Fallback
    }
}

bool RenderMesh::load(const std::string& filename) {
    // Simplified caching for now as RenderFace has dynamic vectors
    Assimp::Importer importer;
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
    hasVertexColors = false;
    hasFaceColors = false;
    
    unsigned int totalVertices = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        totalVertices += scene->mMeshes[i]->mNumVertices;
    }
    vertices.reserve(totalVertices);

    minBB = glm::vec3(std::numeric_limits<float>::max());
    maxBB = glm::vec3(std::numeric_limits<float>::lowest());

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* ai_mesh = scene->mMeshes[i];
        unsigned int baseVertex = static_cast<unsigned int>(vertices.size());

        aiColor4D materialColor(0.8f, 0.8f, 0.8f, 1.0f);
        if (scene->HasMaterials()) {
            aiMaterial* mat = scene->mMaterials[ai_mesh->mMaterialIndex];
            aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &materialColor);
        }

        if (ai_mesh->HasVertexColors(0)) hasVertexColors = true;
        if (materialColor.r != 0.8f || materialColor.g != 0.8f || materialColor.b != 0.8f) hasVertexColors = true;

        for (unsigned int j = 0; j < ai_mesh->mNumVertices; ++j) {
            RenderVertex v;
            const aiVector3D& pos = ai_mesh->mVertices[j];
            v.position = glm::vec3(pos.x, pos.y, pos.z);
            
            if (ai_mesh->HasNormals()) {
                const aiVector3D& norm = ai_mesh->mNormals[j];
                v.normal = glm::vec3(norm.x, norm.y, norm.z);
            } else {
                v.normal = glm::vec3(0.0f);
            }

            if (ai_mesh->HasVertexColors(0)) {
                const aiColor4D& col = ai_mesh->mColors[0][j];
                v.color = glm::vec4(col.r, col.g, col.b, col.a);
            } else {
                v.color = glm::vec4(materialColor.r, materialColor.g, materialColor.b, materialColor.a);
            }
            
            vertices.push_back(v);
            
            minBB = glm::min(minBB, v.position);
            maxBB = glm::max(maxBB, v.position);
        }

        for (unsigned int j = 0; j < ai_mesh->mNumFaces; ++j) {
            const aiFace& face = ai_mesh->mFaces[j];
            RenderFace rf;
            rf.color = glm::vec4(materialColor.r, materialColor.g, materialColor.b, materialColor.a);
            
            if (face.mNumIndices >= 3) {
                glm::vec3 v0 = vertices[baseVertex + face.mIndices[0]].position;
                glm::vec3 v1 = vertices[baseVertex + face.mIndices[1]].position;
                glm::vec3 v2 = vertices[baseVertex + face.mIndices[2]].position;
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                rf.normal = glm::normalize(glm::cross(edge1, edge2));
            }
            
            for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                rf.nodes.push_back(baseVertex + face.mIndices[k]);
            }
            
            faces.push_back(rf);
        }
    }

    return true;
}

void RenderMesh::enableFaceColors() {
    hasFaceColors = true;
}

void RenderMesh::clear() {
    vertices.clear();
    faces.clear();
    edges.clear();
    hasVertexColors = false;
    hasFaceColors = false;
}

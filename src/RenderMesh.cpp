#include "RenderMesh.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <fstream>
#include <filesystem>
#include <random>
#include <utility>
#include <vector>

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
    std::string cacheFilename = getCachePath(filename);
    const uint32_t CACHE_MAGIC = 0x524D5348; // RMSH
    const uint32_t CACHE_VERSION = 1;

    // 1. Try loading from cache
    if (fs::exists(cacheFilename) && fs::exists(filename)) {
        auto sourceTime = fs::last_write_time(filename);
        auto cacheTime = fs::last_write_time(cacheFilename);

        if (cacheTime > sourceTime) {
            std::ifstream is(cacheFilename, std::ios::binary);
            if (is) {
                uint32_t magic, version;
                is.read(reinterpret_cast<char*>(&magic), sizeof(uint32_t));
                is.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

                if (magic == CACHE_MAGIC && version == CACHE_VERSION) {
                    clear();
                    size_t count;

                    // Vertices
                    is.read(reinterpret_cast<char*>(&count), sizeof(size_t));
                    vertices.resize(count);
                    is.read(reinterpret_cast<char*>(vertices.data()), count * sizeof(RenderVertex));

                    // Faces
                    is.read(reinterpret_cast<char*>(&count), sizeof(size_t));
                    faces.resize(count);
                    for (auto& f : faces) {
                        size_t nodeCount;
                        is.read(reinterpret_cast<char*>(&nodeCount), sizeof(size_t));
                        f.nodes.resize(nodeCount);
                        is.read(reinterpret_cast<char*>(f.nodes.data()), nodeCount * sizeof(unsigned int));
                        is.read(reinterpret_cast<char*>(&f.color), sizeof(glm::vec4));
                        is.read(reinterpret_cast<char*>(&f.normal), sizeof(glm::vec3));
                    }

                    // Edges
                    is.read(reinterpret_cast<char*>(&count), sizeof(size_t));
                    edges.resize(count);
                    is.read(reinterpret_cast<char*>(edges.data()), count * sizeof(RenderEdge));

                    // Metadata
                    is.read(reinterpret_cast<char*>(&minBB), sizeof(glm::vec3));
                    is.read(reinterpret_cast<char*>(&maxBB), sizeof(glm::vec3));
                    is.read(reinterpret_cast<char*>(&hasVertexColors), sizeof(bool));
                    is.read(reinterpret_cast<char*>(&hasFaceColors), sizeof(bool));

                    if (is) return true;
                }
            }
        }
    }

    // 2. Fallback to Assimp
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
    unsigned int totalFaces = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        totalVertices += scene->mMeshes[i]->mNumVertices;
        totalFaces += scene->mMeshes[i]->mNumFaces;
    }
    vertices.reserve(totalVertices);
    faces.reserve(totalFaces);

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
            rf.nodes.reserve(face.mNumIndices);
            
            for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                rf.nodes.push_back(baseVertex + face.mIndices[k]);
            }
            
            rf.calculateNormal(vertices);
            faces.push_back(rf);
        }
    }

    generateEdges();

    // 3. Save to cache
    std::ofstream os(cacheFilename, std::ios::binary);
    if (os) {
        os.write(reinterpret_cast<const char*>(&CACHE_MAGIC), sizeof(uint32_t));
        os.write(reinterpret_cast<const char*>(&CACHE_VERSION), sizeof(uint32_t));

        // Vertices
        size_t count = vertices.size();
        os.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
        os.write(reinterpret_cast<const char*>(vertices.data()), count * sizeof(RenderVertex));

        // Faces
        count = faces.size();
        os.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
        for (const auto& f : faces) {
            size_t nodeCount = f.nodes.size();
            os.write(reinterpret_cast<const char*>(&nodeCount), sizeof(size_t));
            os.write(reinterpret_cast<const char*>(f.nodes.data()), nodeCount * sizeof(unsigned int));
            os.write(reinterpret_cast<const char*>(&f.color), sizeof(glm::vec4));
            os.write(reinterpret_cast<const char*>(&f.normal), sizeof(glm::vec3));
        }

        // Edges
        count = edges.size();
        os.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
        os.write(reinterpret_cast<const char*>(edges.data()), count * sizeof(RenderEdge));

        // Metadata
        os.write(reinterpret_cast<const char*>(&minBB), sizeof(glm::vec3));
        os.write(reinterpret_cast<const char*>(&maxBB), sizeof(glm::vec3));
        os.write(reinterpret_cast<const char*>(&hasVertexColors), sizeof(bool));
        os.write(reinterpret_cast<const char*>(&hasFaceColors), sizeof(bool));
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

void RenderMesh::generateEdges() {
    edges.clear();
    if (faces.empty()) return;

    std::vector<std::pair<unsigned int, unsigned int>> tempEdges;
    tempEdges.reserve(faces.size() * 3);

    for (const auto& face : faces) {
        if (face.nodes.size() < 2) continue;
        for (size_t i = 0; i < face.nodes.size(); ++i) {
            unsigned int v0 = face.nodes[i];
            unsigned int v1 = face.nodes[(i + 1) % face.nodes.size()];
            tempEdges.push_back({std::min(v0, v1), std::max(v0, v1)});
        }
    }

    std::sort(tempEdges.begin(), tempEdges.end());
    tempEdges.erase(std::unique(tempEdges.begin(), tempEdges.end()), tempEdges.end());

    edges.reserve(tempEdges.size());
    for (const auto& pair : tempEdges) {
        RenderEdge edge;
        edge.nodes = {pair.first, pair.second};
        edges.push_back(edge);
    }
}

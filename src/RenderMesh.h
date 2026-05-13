#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct RenderVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
};

struct RenderFace {
    std::vector<unsigned int> nodes;
    glm::vec4 color = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);
    glm::vec3 normal;
};

struct RenderEdge {
    std::array<unsigned int, 2> nodes;
    glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float lineThickness = 1.0f;
};

struct RenderMesh {
    std::vector<RenderVertex> vertices;
    std::vector<RenderFace> faces;
    std::vector<RenderEdge> edges;
    
    glm::vec3 minBB;
    glm::vec3 maxBB;

    bool hasVertexColors = false;
    bool hasFaceColors = false;

    bool load(const std::string& filename);
    void clear();


    void enableFaceColors();
    
    static std::string getCachePath(const std::string& filename);
};

#endif

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Mesh {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    glm::vec3 minBB;
    glm::vec3 maxBB;

    bool hasVertexColors = false;

    bool load(const std::string& filename);
    void clear();
    
    static std::string getCachePath(const std::string& filename);
};

#endif

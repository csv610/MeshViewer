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
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    glm::vec3 minBB;
    glm::vec3 maxBB;

    bool load(const std::string& filename);
    void clear();
};

#endif

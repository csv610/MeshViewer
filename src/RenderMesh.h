#ifndef RENDER_MESH_H
#define RENDER_MESH_H

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
    std::vector<glm::vec4> faceColors;
    
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

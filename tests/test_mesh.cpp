#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "RenderMesh.h"
#include <fstream>
#include <filesystem>
#include <numeric>

TEST_CASE("RenderMesh Loading and Bounding Box", "[mesh]") {
    // 1. Create a dummy OBJ file (a simple cube)
    std::string filename = "test_cube.obj";
    std::ofstream out(filename);
    out << "v 0 0 0\n"
        << "v 1 0 0\n"
        << "v 1 1 0\n"
        << "v 0 1 0\n"
        << "v 0 0 1\n"
        << "v 1 0 1\n"
        << "v 1 1 1\n"
        << "v 0 1 1\n"
        << "f 1 2 3\n"
        << "f 5 6 7\n"; // Uses vertices from both Z=0 and Z=1
    out.close();

    RenderMesh m;
    
    SECTION("Load valid mesh") {
        REQUIRE(m.load(filename) == true);
        REQUIRE(m.vertices.size() >= 6); 
        REQUIRE(m.faces.size() == 2);
        size_t totalIndices = std::accumulate(m.faces.begin(), m.faces.end(), 0ULL, 
            [](size_t sum, const RenderFace& f) { return sum + f.nodes.size(); });
        REQUIRE(totalIndices == 6);
    }

    SECTION("Bounding Box Calculation") {
        m.load(filename);
        REQUIRE(m.minBB.x == 0.0f);
        REQUIRE(m.minBB.y == 0.0f);
        REQUIRE(m.minBB.z == 0.0f);
        REQUIRE(m.maxBB.x == 1.0f);
        REQUIRE(m.maxBB.y == 1.0f);
        REQUIRE(m.maxBB.z == 1.0f);
    }

    SECTION("Load invalid mesh") {
        REQUIRE(m.load("non_existent.obj") == false);
    }

    // Cleanup
    std::filesystem::remove(filename);
}

TEST_CASE("Vertex Colors", "[mesh]") {
    // Create a simple PLY file with vertex colors
    std::string filename = "test_colors.ply";
    std::ofstream out(filename);
    out << "ply\n"
        << "format ascii 1.0\n"
        << "element vertex 3\n"
        << "property float x\n"
        << "property float y\n"
        << "property float z\n"
        << "property uchar red\n"
        << "property uchar green\n"
        << "property uchar blue\n"
        << "element face 1\n"
        << "property list uchar int vertex_index\n"
        << "end_header\n"
        << "0 0 0 255 0 0\n"
        << "1 0 0 0 255 0\n"
        << "0 1 0 0 0 255\n"
        << "3 0 1 2\n";
    out.close();

    RenderMesh m;
    REQUIRE(m.load(filename) == true);
    REQUIRE(m.hasVertexColors == true);
    // Assimp might not give exactly 1.0f due to float conversion, but close enough
    REQUIRE(m.vertices[0].color.r == Catch::Approx(1.0f));
    REQUIRE(m.vertices[1].color.g == Catch::Approx(1.0f));
    REQUIRE(m.vertices[2].color.b == Catch::Approx(1.0f));

    std::filesystem::remove(filename);
}

TEST_CASE("RenderMesh Caching Logic", "[mesh]") {
    std::string filename = "dummy.obj";
    std::string cachePath = RenderMesh::getCachePath(filename);
    
    REQUIRE(!cachePath.empty());
    REQUIRE(cachePath.find("meshviewer_") != std::string::npos);
}

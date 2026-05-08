#include <catch2/catch_test_macros.hpp>
#include "../Mesh.h"
#include <fstream>
#include <filesystem>

TEST_CASE("Mesh Loading and Bounding Box", "[mesh]") {
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

    Mesh m;
    
    SECTION("Load valid mesh") {
        REQUIRE(m.load(filename) == true);
        REQUIRE(m.vertices.size() >= 6); // At least vertices used in faces
        REQUIRE(m.indices.size() == 6); // 2 triangles
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

TEST_CASE("Mesh Caching Logic", "[mesh]") {
    std::string filename = "dummy.obj";
    std::string cachePath = Mesh::getCachePath(filename);
    
    REQUIRE(!cachePath.empty());
    REQUIRE(cachePath.find("meshviewer_") != std::string::npos);
}

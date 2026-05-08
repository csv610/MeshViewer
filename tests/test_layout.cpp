#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Viewer.h"
#include <QApplication>

// Helper to ensure QApplication exists
static void ensureApp() {
    static int argc = 1;
    static char* argv[] = {(char*)"test"};
    if (!qApp) {
        new QApplication(argc, argv);
    }
}

TEST_CASE("Viewer Layout Logic", "[layout]") {
    ensureApp();
    Viewer viewer;
    viewer.isTestMode = true;

    Mesh m1;
    m1.minBB = glm::vec3(0, 0, 0);
    m1.maxBB = glm::vec3(10, 10, 10);
    
    Mesh m2;
    m2.minBB = glm::vec3(0, 0, 0);
    m2.maxBB = glm::vec3(5, 5, 5);

    SECTION("Single Mesh Alignment") {
        viewer.clearScene();
        viewer.addMesh(m1, "m1");
        
        auto models = viewer.getModels();
        REQUIRE(models.size() == 1);
        
        // Global BB should be centered at origin
        glm::vec3 min = viewer.getGlobalMinBB();
        glm::vec3 max = viewer.getGlobalMaxBB();
        glm::vec3 center = (min + max) * 0.5f;
        
        CHECK_THAT(center.x, Catch::Matchers::WithinAbs(0.0, 1e-5));
        CHECK_THAT(center.y, Catch::Matchers::WithinAbs(0.0, 1e-5));
        CHECK_THAT(center.z, Catch::Matchers::WithinAbs(0.0, 1e-5));
    }

    SECTION("Multi-Mesh No-Overlap") {
        viewer.clearScene();
        viewer.normalizeScale = true;
        viewer.addMesh(m1, "m1");
        viewer.addMesh(m2, "m2");
        
        auto models = viewer.getModels();
        REQUIRE(models.size() == 2);
        
        auto& mod1 = models[0];
        auto& mod2 = models[1];
        
        float s1 = mod1->scale;
        float s2 = mod2->scale;
        
        float right1 = mod1->offset.x + mod1->data.maxBB.x * s1;
        float left2 = mod2->offset.x + mod2->data.minBB.x * s2;
        
        // Should be at least some padding
        CHECK(left2 > right1);
    }

    SECTION("Normalization Toggle") {
        viewer.clearScene();
        viewer.addMesh(m1, "m1");
        viewer.addMesh(m2, "m2");
        
        viewer.normalizeScale = false;
        viewer.updateLayout();
        
        auto models = viewer.getModels();
        // m1 height is 10, m2 height is 5.
        // Without normalization, they should keep their sizes.
        // (Offset still aligns centers on Y/Z)
        
        viewer.normalizeScale = true;
        viewer.updateLayout();
        // With normalization, both should be 100 units tall
        float s1 = models[0]->scale;
        float h1 = (models[0]->data.maxBB.y - models[0]->data.minBB.y) * s1;
        CHECK_THAT(h1, Catch::Matchers::WithinAbs(100.0, 1e-5));
    }
}

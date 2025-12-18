#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "lsystem/lsystem.h"

class LSystemRenderer {
public:
    enum class PlantType {
        SimplePlant,
        Tree,
        Bush,
        Fern,
        Custom
    };

    void initialize();
    void finish();

    void generatePlant(PlantType type,
                       int iterations,
                       const glm::vec3& position,
                       float scale,
                       float yawRadians = 0.0f,
                       glm::vec3 branchColor = glm::vec3(0.4f, 0.26f, 0.13f),
                       glm::vec3 leafColor = glm::vec3(0.2f, 0.6f, 0.2f));

    void setCustomLSystem(const std::string& axiom,
                          const std::vector<std::pair<char, std::string>>& rules,
                          float angle, float stepLength, int iterations);

    // Render all L-System plants using the provided shader program. The caller is
    // responsible for binding the shader program and providing the correct
    // uniform locations for model and normal matrices.
    void render(GLuint shaderProgram,
                GLint modelLoc,
                GLint normalMatrixLoc,
                const glm::vec3& branchColor = glm::vec3(0.4f, 0.26f, 0.13f),
                const glm::vec3& leafColor = glm::vec3(0.2f, 0.6f, 0.2f));

    void clear();
    bool hasPlants() const { return !m_plants.empty(); }
    size_t getPlantCount() const { return m_plants.size(); }

private:
    struct PlantInstance {
        GLuint branchVao = 0;
        GLuint branchVbo = 0;
        GLuint leafVao = 0;
        GLuint leafVbo = 0;
        int branchVertexCount = 0;
        int leafVertexCount = 0;
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::vec3 branchColor = glm::vec3(0.4f, 0.26f, 0.13f);
        glm::vec3 leafColor = glm::vec3(0.2f, 0.6f, 0.2f);
    };

    void createPlantBuffers(PlantInstance& plant,
                            const std::vector<float>& branchVertices,
                            const std::vector<float>& leafVertices);
    void destroyPlantBuffers(PlantInstance& plant);

    std::vector<PlantInstance> m_plants;
    bool m_initialized = false;
};

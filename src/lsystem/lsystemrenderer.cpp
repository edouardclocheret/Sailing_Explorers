#include "lsystem/lsystemrenderer.h"

#include <glm/gtc/matrix_transform.hpp>

void LSystemRenderer::initialize() {
    if (m_initialized) return;
    m_initialized = true;
}

void LSystemRenderer::finish() {
    if (!m_initialized) return;

    for (auto& plant : m_plants) {
        destroyPlantBuffers(plant);
    }
    m_plants.clear();

    m_initialized = false;
}

void LSystemRenderer::generatePlant(PlantType type,
                                    int iterations,
                                    const glm::vec3& position,
                                    float scale,
                                    float yawRadians,
                                    glm::vec3 branchColor,
                                    glm::vec3 leafColor) {
    if (!m_initialized) return;

    LSystem lsystem;

    switch (type) {
        case PlantType::SimplePlant:
            lsystem = LSystem::createSimplePlant();
            break;
        case PlantType::Tree:
            lsystem = LSystem::createTree();
            break;
        case PlantType::Bush:
            lsystem = LSystem::createBush();
            break;
        case PlantType::Fern:
            lsystem = LSystem::createFern();
            break;
        default:
            lsystem = LSystem::createSimplePlant();
            break;
    }

    std::string lstring = lsystem.generate(iterations);
    lsystem.interpret(lstring);

    std::vector<float> branchVertices = lsystem.generateBranchMesh(6);
    std::vector<float> leafVertices = lsystem.generateLeafMesh();

    PlantInstance plant;
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, yawRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    model = model * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    plant.modelMatrix = model;
    plant.branchColor = branchColor;
    plant.leafColor = leafColor;

    createPlantBuffers(plant, branchVertices, leafVertices);
    m_plants.push_back(plant);
}

void LSystemRenderer::setCustomLSystem(const std::string& axiom,
                                      const std::vector<std::pair<char, std::string>>& rules,
                                      float angle, float stepLength, int iterations) {
    (void)axiom; (void)rules; (void)angle; (void)stepLength; (void)iterations;
}

void LSystemRenderer::render(GLuint shaderProgram, GLint modelLoc, GLint normalMatrixLoc, const glm::vec3& branchTint, const glm::vec3& leafTint) {
    if (!m_initialized || m_plants.empty()) return;

    // Use shader program already bound by caller
    // Uniforms: shader expects cAmbient/cDiffuse/cSpecular and n for shininess

    for (const auto& plant : m_plants) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &plant.modelMatrix[0][0]);

        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(plant.modelMatrix)));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

        // Branch color
        GLint cAmbLoc = glGetUniformLocation(shaderProgram, "cAmbient");
        GLint cDifLoc = glGetUniformLocation(shaderProgram, "cDiffuse");
        GLint cSpeLoc = glGetUniformLocation(shaderProgram, "cSpecular");
        GLint nLoc = glGetUniformLocation(shaderProgram, "n");

        if (plant.branchVao != 0 && plant.branchVertexCount > 0) {
            glm::vec3 branchCol = plant.branchColor * branchTint;
            glUniform3fv(cAmbLoc, 1, &branchCol[0]);
            glUniform3fv(cDifLoc, 1, &branchCol[0]);
            glm::vec3 spec(0.2f,0.2f,0.2f);
            glUniform3fv(cSpeLoc, 1, &spec[0]);
            glUniform1f(nLoc, 16.0f);

            glBindVertexArray(plant.branchVao);
            glDrawArrays(GL_TRIANGLES, 0, plant.branchVertexCount);
        }

        if (plant.leafVao != 0 && plant.leafVertexCount > 0) {
            glm::vec3 leafCol = plant.leafColor * leafTint;
            glUniform3fv(cAmbLoc, 1, &leafCol[0]);
            glUniform3fv(cDifLoc, 1, &leafCol[0]);
            glm::vec3 leafSpec(0.1f,0.1f,0.1f);
            glUniform3fv(cSpeLoc, 1, &leafSpec[0]);
            glUniform1f(nLoc, 8.0f);

            glBindVertexArray(plant.leafVao);
            glDrawArrays(GL_TRIANGLES, 0, plant.leafVertexCount);
        }
    }

    glBindVertexArray(0);
}

void LSystemRenderer::clear() {
    for (auto& plant : m_plants) {
        destroyPlantBuffers(plant);
    }
    m_plants.clear();
}

void LSystemRenderer::createPlantBuffers(PlantInstance& plant, const std::vector<float>& branchVertices, const std::vector<float>& leafVertices) {
    if (!branchVertices.empty()) {
        glGenVertexArrays(1, &plant.branchVao);
        glGenBuffers(1, &plant.branchVbo);

        glBindVertexArray(plant.branchVao);
        glBindBuffer(GL_ARRAY_BUFFER, plant.branchVbo);
        glBufferData(GL_ARRAY_BUFFER, branchVertices.size() * sizeof(float), branchVertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        plant.branchVertexCount = static_cast<int>(branchVertices.size() / 6);
        glBindVertexArray(0);
    }

    if (!leafVertices.empty()) {
        glGenVertexArrays(1, &plant.leafVao);
        glGenBuffers(1, &plant.leafVbo);

        glBindVertexArray(plant.leafVao);
        glBindBuffer(GL_ARRAY_BUFFER, plant.leafVbo);
        glBufferData(GL_ARRAY_BUFFER, leafVertices.size() * sizeof(float), leafVertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        plant.leafVertexCount = static_cast<int>(leafVertices.size() / 6);
        glBindVertexArray(0);
    }
}

void LSystemRenderer::destroyPlantBuffers(PlantInstance& plant) {
    if (plant.branchVbo != 0) {
        glDeleteBuffers(1, &plant.branchVbo);
        plant.branchVbo = 0;
    }
    if (plant.branchVao != 0) {
        glDeleteVertexArrays(1, &plant.branchVao);
        plant.branchVao = 0;
    }
    if (plant.leafVbo != 0) {
        glDeleteBuffers(1, &plant.leafVbo);
        plant.leafVbo = 0;
    }
    if (plant.leafVao != 0) {
        glDeleteVertexArrays(1, &plant.leafVao);
        plant.leafVao = 0;
    }
    plant.branchVertexCount = 0;
    plant.leafVertexCount = 0;
}

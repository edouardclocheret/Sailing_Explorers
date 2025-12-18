#include "render/instancedtrees.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

namespace {
constexpr int kCylinderSegments = 20;
constexpr float kTrunkHeight = 1.0f;
constexpr float kTrunkRadius = 0.12f;
constexpr float kCanopyRadius = 0.55f;
constexpr float kCanopyHeight = 1.2f;
constexpr float kCanopyBaseY = kTrunkHeight;
}

void InstancedTreeRenderer::initialize() {
    if (m_initialized) {
        return;
    }

    std::vector<float> trunkData = generateTrunkGeometry();
    std::vector<float> canopyData = generateCanopyGeometry();

    glGenVertexArrays(1, &m_trunkVAO);
    glBindVertexArray(m_trunkVAO);

    glGenBuffers(1, &m_trunkVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_trunkVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 trunkData.size() * sizeof(float),
                 trunkData.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glGenVertexArrays(1, &m_canopyVAO);
    glBindVertexArray(m_canopyVAO);

    glGenBuffers(1, &m_canopyVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_canopyVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 canopyData.size() * sizeof(float),
                 canopyData.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glGenBuffers(1, &m_instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // Attach instanced attributes to both VAOs
    setupInstanceAttributes(m_trunkVAO);
    setupInstanceAttributes(m_canopyVAO);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_trunkVertexCount = static_cast<GLsizei>(trunkData.size() / 6);
    m_canopyVertexCount = static_cast<GLsizei>(canopyData.size() / 6);

    m_initialized = true;
}

void InstancedTreeRenderer::finish() {
    if (!m_initialized) {
        return;
    }

    glDeleteVertexArrays(1, &m_trunkVAO);
    glDeleteVertexArrays(1, &m_canopyVAO);
    glDeleteBuffers(1, &m_trunkVBO);
    glDeleteBuffers(1, &m_canopyVBO);
    glDeleteBuffers(1, &m_instanceVBO);

    m_trunkVAO = 0;
    m_canopyVAO = 0;
    m_trunkVBO = 0;
    m_canopyVBO = 0;
    m_instanceVBO = 0;

    m_trunkVertexCount = 0;
    m_canopyVertexCount = 0;
    m_instanceCount = 0;
    m_initialized = false;
}

void InstancedTreeRenderer::updateInstances(const std::vector<glm::mat4> &models) {
    if (!m_initialized) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 models.size() * sizeof(glm::mat4),
                 models.empty() ? nullptr : models.data(),
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_instanceCount = static_cast<GLsizei>(models.size());
}

void InstancedTreeRenderer::clearInstances() {
    if (!m_initialized) {
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_instanceCount = 0;
}

void InstancedTreeRenderer::drawTrunks() const {
    if (m_trunkVAO == 0 || m_trunkVertexCount == 0 || m_instanceCount == 0) {
        return;
    }
    glBindVertexArray(m_trunkVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, m_trunkVertexCount, m_instanceCount);
    glBindVertexArray(0);
}

void InstancedTreeRenderer::drawCanopies() const {
    if (m_canopyVAO == 0 || m_canopyVertexCount == 0 || m_instanceCount == 0) {
        return;
    }
    glBindVertexArray(m_canopyVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, m_canopyVertexCount, m_instanceCount);
    glBindVertexArray(0);
}

std::vector<float> InstancedTreeRenderer::generateTrunkGeometry() const {
    std::vector<float> data;
    data.reserve(kCylinderSegments * 6 * 6);

    const float step = glm::two_pi<float>() / static_cast<float>(kCylinderSegments);

    for (int i = 0; i < kCylinderSegments; ++i) {
        float theta = i * step;
        float nextTheta = (i + 1) * step;

        glm::vec3 p0(kTrunkRadius * std::cos(theta), 0.0f, kTrunkRadius * std::sin(theta));
        glm::vec3 p1(kTrunkRadius * std::cos(nextTheta), 0.0f, kTrunkRadius * std::sin(nextTheta));
        glm::vec3 p2(kTrunkRadius * std::cos(nextTheta), kTrunkHeight, kTrunkRadius * std::sin(nextTheta));
        glm::vec3 p3(kTrunkRadius * std::cos(theta), kTrunkHeight, kTrunkRadius * std::sin(theta));

        glm::vec3 normal0 = glm::normalize(glm::vec3(std::cos(theta), 0.0f, std::sin(theta)));
        glm::vec3 normal1 = glm::normalize(glm::vec3(std::cos(nextTheta), 0.0f, std::sin(nextTheta)));

        auto pushVertex = [&data](const glm::vec3 &pos, const glm::vec3 &normal) {
            data.push_back(pos.x);
            data.push_back(pos.y);
            data.push_back(pos.z);
            data.push_back(normal.x);
            data.push_back(normal.y);
            data.push_back(normal.z);
        };

        // First triangle p0 p1 p2
        pushVertex(p0, normal0);
        pushVertex(p1, normal1);
        pushVertex(p2, normal1);
        // Second triangle p0 p2 p3
        pushVertex(p0, normal0);
        pushVertex(p2, normal1);
        pushVertex(p3, normal0);
    }

    // Bottom cap
    for (int i = 0; i < kCylinderSegments; ++i) {
        float theta = i * step;
        float nextTheta = (i + 1) * step;

        glm::vec3 center(0.0f, 0.0f, 0.0f);
        glm::vec3 p0(kTrunkRadius * std::cos(theta), 0.0f, kTrunkRadius * std::sin(theta));
        glm::vec3 p1(kTrunkRadius * std::cos(nextTheta), 0.0f, kTrunkRadius * std::sin(nextTheta));

        glm::vec3 normal(0.0f, -1.0f, 0.0f);
        auto pushVertex = [&data](const glm::vec3 &pos, const glm::vec3 &normal) {
            data.insert(data.end(), {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z});
        };

        pushVertex(center, normal);
        pushVertex(p1, normal);
        pushVertex(p0, normal);
    }

    // Top cap
    for (int i = 0; i < kCylinderSegments; ++i) {
        float theta = i * step;
        float nextTheta = (i + 1) * step;

        glm::vec3 center(0.0f, kTrunkHeight, 0.0f);
        glm::vec3 p0(kTrunkRadius * std::cos(theta), kTrunkHeight, kTrunkRadius * std::sin(theta));
        glm::vec3 p1(kTrunkRadius * std::cos(nextTheta), kTrunkHeight, kTrunkRadius * std::sin(nextTheta));

        glm::vec3 normal(0.0f, 1.0f, 0.0f);
        auto pushVertex = [&data](const glm::vec3 &pos, const glm::vec3 &normal) {
            data.insert(data.end(), {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z});
        };

        pushVertex(center, normal);
        pushVertex(p0, normal);
        pushVertex(p1, normal);
    }

    return data;
}

std::vector<float> InstancedTreeRenderer::generateCanopyGeometry() const {
    std::vector<float> data;
    data.reserve(kCylinderSegments * 6 * 3);

    const float step = glm::two_pi<float>() / static_cast<float>(kCylinderSegments);
    const float tipY = kCanopyBaseY + kCanopyHeight;

    for (int i = 0; i < kCylinderSegments; ++i) {
        float theta = i * step;
        float nextTheta = (i + 1) * step;

        glm::vec3 p0(kCanopyRadius * std::cos(theta), kCanopyBaseY, kCanopyRadius * std::sin(theta));
        glm::vec3 p1(kCanopyRadius * std::cos(nextTheta), kCanopyBaseY, kCanopyRadius * std::sin(nextTheta));
        glm::vec3 tip(0.0f, tipY, 0.0f);

        glm::vec3 normal0 = glm::normalize(glm::vec3(std::cos(theta), kCanopyRadius / kCanopyHeight, std::sin(theta)));
        glm::vec3 normal1 = glm::normalize(glm::vec3(std::cos(nextTheta), kCanopyRadius / kCanopyHeight, std::sin(nextTheta)));
        glm::vec3 normalTip = glm::normalize(normal0 + normal1);

        auto pushVertex = [&data](const glm::vec3 &pos, const glm::vec3 &normal) {
            data.push_back(pos.x);
            data.push_back(pos.y);
            data.push_back(pos.z);
            data.push_back(normal.x);
            data.push_back(normal.y);
            data.push_back(normal.z);
        };

        pushVertex(p0, normal0);
        pushVertex(p1, normal1);
        pushVertex(tip, normalTip);
    }

    // Base disk to close canopy
    for (int i = 0; i < kCylinderSegments; ++i) {
        float theta = i * step;
        float nextTheta = (i + 1) * step;

        glm::vec3 center(0.0f, kCanopyBaseY, 0.0f);
        glm::vec3 p0(kCanopyRadius * std::cos(theta), kCanopyBaseY, kCanopyRadius * std::sin(theta));
        glm::vec3 p1(kCanopyRadius * std::cos(nextTheta), kCanopyBaseY, kCanopyRadius * std::sin(nextTheta));

        glm::vec3 normal(0.0f, -1.0f, 0.0f);
        auto pushVertex = [&data](const glm::vec3 &pos, const glm::vec3 &normal) {
            data.insert(data.end(), {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z});
        };

        pushVertex(center, normal);
        pushVertex(p0, normal);
        pushVertex(p1, normal);
    }

    return data;
}

void InstancedTreeRenderer::setupInstanceAttributes(GLuint vao) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(vec4Size * i));
        glVertexAttribDivisor(2 + i, 1);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

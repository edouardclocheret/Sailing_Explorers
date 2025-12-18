#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>

class InstancedTreeRenderer {
public:
    void initialize();
    void finish();

    void updateInstances(const std::vector<glm::mat4> &models);
    void clearInstances();

    void drawTrunks() const;
    void drawCanopies() const;

    GLsizei getInstanceCount() const { return m_instanceCount; }

private:
    std::vector<float> generateTrunkGeometry() const;
    std::vector<float> generateCanopyGeometry() const;
    void setupInstanceAttributes(GLuint vao);

    GLuint m_trunkVAO = 0;
    GLuint m_trunkVBO = 0;
    GLuint m_canopyVAO = 0;
    GLuint m_canopyVBO = 0;
    GLuint m_instanceVBO = 0;

    GLsizei m_trunkVertexCount = 0;
    GLsizei m_canopyVertexCount = 0;
    GLsizei m_instanceCount = 0;

    bool m_initialized = false;
};

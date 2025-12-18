#include "render/postprocessor.h"

#include <algorithm>
#include <iostream>

#include <glm/vec2.hpp>

#include "utils/shaderloader.h"

namespace {
constexpr const char *kPostProcessVertex = ":/resources/shaders/postprocess.vert";
constexpr const char *kPostProcessFragment = ":/resources/shaders/postprocess.frag";
}

void PostProcessor::initialize() {
    if (m_initialized) {
        return;
    }

    loadShader();
    createQuad();
    cacheUniformLocations();
    m_initialized = true;
}

void PostProcessor::finish() {
    if (!m_initialized) {
        return;
    }

    destroyQuad();
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }

    m_initialized = false;
}

void PostProcessor::render(GLuint colorTexture,
                           GLuint depthTexture,
                           float nearPlane,
                           float farPlane,
                           const glm::ivec2 &viewportSize,
                           bool visualizeDepth,
                           const DOFParams &dofParams) {
    if (!m_initialized || colorTexture == 0) {
        return;
    }

    bool showDepth = visualizeDepth && depthTexture != 0;

    glDisable(GL_DEPTH_TEST);
    glUseProgram(m_shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glUniform1i(m_locColorTex, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(m_locDepthTex, 1);

    glUniform1i(m_locMode, showDepth ? 1 : 0);
    glUniform1f(m_locNear, nearPlane);
    glUniform1f(m_locFar, farPlane);

    bool dofActive = dofParams.enabled && depthTexture != 0;
    glUniform1i(m_locDOFEnabled, dofActive ? 1 : 0);
    glUniform1f(m_locDOFFocalDist, dofParams.focalDistance);
    glUniform1f(m_locDOFAperture, dofParams.aperture);

    float texelX = 1.0f / std::max(viewportSize.x, 1);
    float texelY = 1.0f / std::max(viewportSize.y, 1);
    glUniform2f(m_locTexelSize, texelX, texelY);

    glViewport(0, 0, std::max(viewportSize.x, 1), std::max(viewportSize.y, 1));
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(m_quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
}

void PostProcessor::createQuad() {
    GLfloat quadVertices[] = {
        -1.f, -1.f, 0.f, 0.f,
         1.f, -1.f, 1.f, 0.f,
         1.f,  1.f, 1.f, 1.f,

        -1.f, -1.f, 0.f, 0.f,
         1.f,  1.f, 1.f, 1.f,
        -1.f,  1.f, 0.f, 1.f
    };

    glGenVertexArrays(1, &m_quadVao);
    glGenBuffers(1, &m_quadVbo);

    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<void*>(2 * sizeof(GLfloat)));

    glBindVertexArray(0);
}

void PostProcessor::destroyQuad() {
    if (m_quadVbo != 0) {
        glDeleteBuffers(1, &m_quadVbo);
        m_quadVbo = 0;
    }
    if (m_quadVao != 0) {
        glDeleteVertexArrays(1, &m_quadVao);
        m_quadVao = 0;
    }
}

void PostProcessor::loadShader() {
    m_shaderProgram = ShaderLoader::createShaderProgram(kPostProcessVertex, kPostProcessFragment);
}

void PostProcessor::cacheUniformLocations() {
    m_locColorTex = glGetUniformLocation(m_shaderProgram, "uColorTex");
    m_locDepthTex = glGetUniformLocation(m_shaderProgram, "uDepthTex");
    m_locMode = glGetUniformLocation(m_shaderProgram, "uMode");
    m_locNear = glGetUniformLocation(m_shaderProgram, "uNear");
    m_locFar = glGetUniformLocation(m_shaderProgram, "uFar");

    m_locDOFEnabled = glGetUniformLocation(m_shaderProgram, "uDofEnabled");
    m_locDOFFocalDist = glGetUniformLocation(m_shaderProgram, "uFocalDistance");
    m_locDOFAperture = glGetUniformLocation(m_shaderProgram, "uAperture");
    m_locTexelSize = glGetUniformLocation(m_shaderProgram, "uTexelSize");
}

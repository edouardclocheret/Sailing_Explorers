#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>

#include <glm/glm.hpp>

// DOF params
struct DOFParams {
    bool enabled = false;
    float focalDistance = 5.0f;
    float aperture = 0.1f;
    float maxBlur = 10.0f;
};

class PostProcessor {
public:
    void initialize();
    void finish();

    // Render accepts color texture and depth texture plus camera near/far
    void render(GLuint colorTexture,
                GLuint depthTexture,
                float nearPlane,
                float farPlane,
                const glm::ivec2 &viewportSize,
                bool visualizeDepth,
                const DOFParams &dofParams = DOFParams{});

private:
    void createQuad();
    void destroyQuad();
    void loadShader();
    void cacheUniformLocations();

    GLuint m_shaderProgram = 0;
    GLuint m_quadVao = 0;
    GLuint m_quadVbo = 0;

    GLint m_locColorTex = -1;
    GLint m_locDepthTex = -1;
    GLint m_locMode = -1;
    GLint m_locNear = -1;
    GLint m_locFar = -1;

    // DOF uniforms
    GLint m_locDOFEnabled = -1;
    GLint m_locDOFFocalDist = -1;
    GLint m_locDOFAperture = -1;
    GLint m_locTexelSize = -1;

    bool m_initialized = false;
};

#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include "settings.h"
#include "src/shapes/Cone.h"
#include "src/shapes/Cube.h"
#include "src/shapes/Cylinder.h"
#include "src/shapes/Sphere.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

void Realtime::updateProjMat(){
    float n = settings.nearPlane;
    float f = settings.farPlane;
    float c = -n/f;

    float thetaH = m_renderData.cameraData.heightAngle;
    float aspectRatio = (float)width() /(float)height();


    glm::mat4 gl_convention(1.f,0.f,0.f,0.f,
                            0.f,1.f,0.f,0.f,
                            0.f,0.f,-2.f,0.f,
                            0.f,0.f,-1.f,1.f);

    glm::mat4 unhinge(
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f/(1.f+c), -1.f,
        0.f, 0.f, -c/(1.f+c), 0.f
        );

    glm::mat4 scaling_perspective(
        1.f / (f*aspectRatio * tan(thetaH * 0.5f)), 0, 0, 0,
        0, 1.f / (f*tan(thetaH * 0.5f)), 0, 0,
        0, 0, 1.f/f, 0,
        0, 0, 0, 1.f
        );


    m_proj = gl_convention*unhinge*scaling_perspective;

}

void Realtime::translateCamera(glm::vec3 dir){


    glm::mat4 M_trans = glm::mat4(1.f,0.f,0.f,0.f,
                                  0.f,1.f,0.f,0.f,
                                  0.f,0.f,1.f,0.f,
                                  dir.x,dir.y,dir.z,1.f);
    m_renderData.cameraData.pos = M_trans * m_renderData.cameraData.pos;
    m_renderData.cameraData.look = M_trans * m_renderData.cameraData.look;
    m_renderData.cameraData.up = M_trans * m_renderData.cameraData.up;
    updateViewMat();
}

void Realtime::updateBoatShapes() {
    for (RenderShapeData &shape : m_renderData.shapes) {
        if (!shape.is_land) {
            shape.ctm = m_boatCTM * shape.local_ctm;
        }
    }
}

void Realtime::rotateCamera(float angleRad, glm::vec3 axis) {
    axis = glm::normalize(axis);
    float c = cos(angleRad);
    float s = sin(angleRad);
    float t = 1 - c;

    glm::mat4 M_rot(
        t*axis.x*axis.x + c,         t*axis.x*axis.y - s*axis.z,   t*axis.x*axis.z + s*axis.y,  0,
        t*axis.x*axis.y + s*axis.z,  t*axis.y*axis.y + c,          t*axis.y*axis.z - s*axis.x,  0,
        t*axis.x*axis.z - s*axis.y,  t*axis.y*axis.z + s*axis.x,   t*axis.z*axis.z + c,         0,
        0,                           0,                            0,                           1
        );

    m_renderData.cameraData.look = M_rot * m_renderData.cameraData.look;
    m_renderData.cameraData.up   = M_rot * m_renderData.cameraData.up;

    updateViewMat();
}

glm::vec3 Realtime::rotateVector(glm::vec3 vec, float angleRad, glm::vec3 axis) {
    axis = glm::normalize(axis);
    float c = cos(angleRad);
    float s = sin(angleRad);
    float t = 1 - c;

    glm::mat4 M_rot(
        t*axis.x*axis.x + c,         t*axis.x*axis.y - s*axis.z,   t*axis.x*axis.z + s*axis.y,  0,
        t*axis.x*axis.y + s*axis.z,  t*axis.y*axis.y + c,          t*axis.y*axis.z - s*axis.x,  0,
        t*axis.x*axis.z - s*axis.y,  t*axis.y*axis.z + s*axis.x,   t*axis.z*axis.z + c,         0,
        0,                           0,                            0,                           1
        );

    return M_rot*glm::vec4(vec,1.0);
}




void Realtime::updateViewMat(){
    SceneCameraData camData = m_renderData.cameraData;
    m_camera = Camera(camData.pos, camData.look, camData.up);
    m_view = m_camera.viewMat;

}

std::vector<float> Realtime::generateShapeData(RenderShapeData& shape_to_add,
                                               GLuint param1, GLuint param2)
{
    std::unique_ptr<Shape> shape;

    switch (shape_to_add.primitive.type) {
    case PrimitiveType::PRIMITIVE_CUBE:
        shape = std::make_unique<Cube>();
        break;
    case PrimitiveType::PRIMITIVE_CONE:
        shape = std::make_unique<Cone>();
        break;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        shape = std::make_unique<Cylinder>();
        break;
    case PrimitiveType::PRIMITIVE_SPHERE:
        shape = std::make_unique<Sphere>();
        break;
    default:
        return {};
    }
    shape->setVertexData(param1, param2);

    return shape->m_vertexData;
}

std::vector<GLfloat> Realtime::generateFloorData(float size) {
    const float nx = 0.0f, ny = 1.0f, nz = 0.0f;

    return {
        //   position          normal            uv
        size, 0.0f,  size,     nx, ny, nz,      1.0f, 1.0f,
        size, 0.0f, -size,     nx, ny, nz,      1.0f, 0.0f,
        -size, 0.0f, -size,     nx, ny, nz,      0.0f, 0.0f,

        -size, 0.0f,  size,     nx, ny, nz,      0.0f, 1.0f,
        size, 0.0f,  size,     nx, ny, nz,      1.0f, 1.0f,
        -size, 0.0f, -size,     nx, ny, nz,      0.0f, 0.0f
    };
}



void Realtime::initializeFloor(){
    glGenVertexArrays(1, &m_floorVAO);
    glGenBuffers(1, &m_floorVBO);

    glBindVertexArray(m_floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_floorVBO);

    auto floorData = generateFloorData(200.0f);
    glBufferData(GL_ARRAY_BUFFER, floorData.size() * sizeof(GLfloat),
                 floorData.data(), GL_STATIC_DRAW);

    const GLsizei stride = 8 * sizeof(GLfloat);

    // Position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    // Normal (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(3 * sizeof(GLfloat)));

    // UV (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)(6 * sizeof(GLfloat)));

    glBindVertexArray(0);

    QImage img(":/resources/textures/water-normal-texture-3.jpg");
    if (img.isNull()) {
        std::cerr << "Failed to load floor texture!" << std::endl;
    }
    img = img.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &m_floorTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_floorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}



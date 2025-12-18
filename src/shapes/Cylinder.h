#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shape.h"

class Cylinder : public Shape
{
public:
    void updateParams(int param1, int param2);
    void setVertexData(GLuint p1, GLuint p2) override {
        updateParams(p1,p2);
        makeCylinder();
    }
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();


    void makeCapTile(bool up, glm::vec3 topLeft,
                     glm::vec3 topRight,
                     glm::vec3 bottomLeft,
                     glm::vec3 bottomRight);
    void makeCapSlice(float currentTheta, float nextTheta);

    glm::vec3 calcNorm(glm::vec3& pt);
    void makeSideTile(glm::vec3 topLeft,
                       glm::vec3 topRight,
                       glm::vec3 bottomLeft,
                       glm::vec3 bottomRight);
    void makeSideSlice(float currentTheta, float nextTheta);
    void makeWedge(float currentTheta, float nextTheta);
    void makeCylinder();


    float m_radius = 0.5;


};

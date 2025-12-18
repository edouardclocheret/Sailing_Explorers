#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shape.h"

class Sphere : public Shape
{
public:
    void updateParams(int param1, int param2);
    void setVertexData(GLuint p1, GLuint p2) override {
        updateParams(p1,p2);
        makeSphere();
    }
    std::vector<float> generateShape() {
        return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeWedge(float currTheta, float nextTheta);

    void makeSphere();

    float m_radius = 0.5;


};

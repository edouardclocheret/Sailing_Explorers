#pragma once

#include <vector>
#include <glm/glm.hpp>

class Tetrahedron
{
public:
    void updateParams(int param1, int param2);
    //std::vector<float> makeTet() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();

    std::vector<float> m_vertexData;

    int m_param1;
    int m_param2;
    glm::vec3 m_color;

};

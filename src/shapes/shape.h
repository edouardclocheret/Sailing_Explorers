#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>


class Shape {
public:
    // These attributes are shared over all shapes

    int m_param1;
    int m_param2;

    std::vector<float> m_vertexData;


    virtual ~Shape() = default;

    // Shapes must generate their geometry
    virtual void setVertexData(GLuint p1, GLuint p2) = 0;

};

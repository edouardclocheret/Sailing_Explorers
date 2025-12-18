#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = std::max(param1,1);
    m_param2 = std::max(param2,3);
}

void Cone::makeCapTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {

    glm::vec3 down(0.f, -1.f, 0.f);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, down);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, down);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, down);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, down);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, down);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, down);
}


glm::vec3 Cone::calcNorm(glm::vec3& pt) {
    float xNorm = (2 * pt.x);
    float yNorm = -(1.f/4.f) * (2.f * pt.y - 1.f);
    float zNorm = (2 * pt.z);
    return glm::normalize(glm::vec3{ xNorm, yNorm, zNorm });
}

void Cone::makeSlopeTile(glm::vec3 topLeft,
                         glm::vec3 topRight,
                         glm::vec3 bottomLeft,
                         glm::vec3 bottomRight) {

    glm::vec3 nBottomLeft = calcNorm(bottomLeft);
    glm::vec3 nBottomRight = calcNorm(bottomRight);
    glm::vec3 nTopLeft = calcNorm(topLeft);
    glm::vec3 nTopRight = calcNorm(topRight);

    //the tip
    const float EPSILON = 1e-5f;
    if (fabs(topLeft.y - 0.5f) < EPSILON) {
        nTopLeft = glm::normalize((nBottomLeft + nBottomRight) * 0.5f);
    }
    if (fabs(topRight.y - 0.5f) < EPSILON) {
        nTopRight = glm::normalize((nBottomLeft + nBottomRight) * 0.5f);
    }

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, nBottomLeft);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, nTopLeft);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, nBottomRight);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, nBottomRight);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, nTopLeft);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, nTopRight);
}


void Cone::makeSlopeSlice(float currentTheta, float nextTheta){
    float y_step = 1.0f / m_param1;
    float y_base = -0.5f;
    float y_top = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float y1 = y_base + i * y_step;
        float y2 = y_base + (i + 1) * y_step;

        float r1 = 0.5f * (y_top - y1) / (y_top - y_base);
        float r2 = 0.5f * (y_top - y2) / (y_top - y_base);

        glm::vec3 topLeft(
            r2 * cos(currentTheta),
            y2,
            r2 * sin(currentTheta));

        glm::vec3 topRight(
            r2 * cos(nextTheta),
            y2,
            r2 * sin(nextTheta));

        glm::vec3 bottomLeft(
            r1 * cos(currentTheta),
            y1,
            r1 * sin(currentTheta));

        glm::vec3 bottomRight(
            r1 * cos(nextTheta),
            y1,
            r1 * sin(nextTheta));
        makeSlopeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cone::makeCapSlice(float currentTheta, float nextTheta){
    float r_step = 0.5f /(float)m_param1;
    float radius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float r1 = i * r_step;
        float r2 = (i + 1) * r_step;

        glm::vec3 topLeft(
            r2 * cos(currentTheta),
            -0.5f,
            r2 * sin(currentTheta));

        glm::vec3 topRight(
            r2 * cos(nextTheta),
            -0.5f,
            r2 * sin(nextTheta));

        glm::vec3 bottomLeft(
            r1 * cos(currentTheta),
            -0.5f,
            r1 * sin(currentTheta));

        glm::vec3 bottomRight(
            r1 * cos(nextTheta),
            -0.5f,
            r1 * sin(nextTheta));
        makeCapTile(topLeft, topRight, bottomLeft, bottomRight);
    }

}

void Cone::makeWedge(float currentTheta, float nextTheta){
    makeCapSlice(currentTheta, nextTheta);
    makeSlopeSlice(currentTheta, nextTheta);
}

void Cone::makeCone(){
    float theta_step = 2*M_PI/(float)m_param2;
    for (int j=0; j<m_param2; j++){
        float currentTheta = j*theta_step;
        float nextTheta = (j+1)*theta_step;
        makeWedge(currentTheta, nextTheta);
    }
}





// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = std::max(param1,1);
    m_param2 = std::max(param2,3);
}

void Cylinder::makeCapTile(bool up, glm::vec3 topLeft,
                       glm::vec3 topRight,
                       glm::vec3 bottomLeft,
                       glm::vec3 bottomRight) {

    glm::vec3 normal(0.f, -1.f, 0.f);//default down
    if (up){
        normal.y = 1.f;
    }
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);


    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
}

glm::vec3 Cylinder::calcNorm(glm::vec3& pt) {
    return glm::normalize(glm::vec3{ pt.x, 0.f, pt.z});
}

void Cylinder::makeSideTile(glm::vec3 topLeft,
                         glm::vec3 topRight,
                         glm::vec3 bottomLeft,
                         glm::vec3 bottomRight) {

    glm::vec3 nBottomLeft = calcNorm(bottomLeft);
    glm::vec3 nBottomRight = calcNorm(bottomRight);
    glm::vec3 nTopLeft = calcNorm(topLeft);
    glm::vec3 nTopRight = calcNorm(topRight);

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


void Cylinder::makeSideSlice(float currentTheta, float nextTheta){

    float y_step = 1.0f / m_param1;
    float y_base = -0.5f;
    float y_top = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float y1 = y_base + i * y_step;
        float y2 = y_base + (i + 1) * y_step;

        float r1 = 0.5f;
        float r2 = 0.5f;

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
        makeSideTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cylinder::makeCapSlice(float currentTheta, float nextTheta){
    float r_step = 0.5f /(float)m_param1;
    float radius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float r1 = i * r_step;
        float r2 = (i + 1) * r_step;
        bool up =true;

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
        makeCapTile(!up, topLeft, topRight, bottomLeft, bottomRight);


        //same for other cap
        topLeft= glm::vec3(
            r2 * cos(currentTheta),
            0.5f,
            r2 * sin(currentTheta));

        topRight= glm::vec3(
            r2 * cos(nextTheta),
            0.5f,
            r2 * sin(nextTheta));

        bottomLeft= glm::vec3(
            r1 * cos(currentTheta),
            0.5f,
            r1 * sin(currentTheta));

        bottomRight = glm::vec3(
            r1 * cos(nextTheta),
            0.5f,
            r1 * sin(nextTheta));
        makeCapTile(up, topLeft, bottomLeft, topRight,bottomRight);
    }

}

void Cylinder::makeWedge(float currentTheta, float nextTheta){
    bool up = true;
    makeCapSlice(currentTheta, nextTheta);
    makeSideSlice(currentTheta, nextTheta);
}

void Cylinder::makeCylinder(){
    float theta_step = 2*M_PI/(float)m_param2;
    for (int j=0; j<m_param2; j++){
        float currentTheta = j*theta_step;
        float nextTheta = (j+1)*theta_step;
        makeWedge(currentTheta, nextTheta);
    }
}

void Cylinder::setVertexData() {
    // TODO for Project 5: Lights, Camera

    //to test the slice
    // float thetaStep = glm::radians(360.f / m_param2);
    // float currentTheta = 0 * thetaStep;
    // float nextTheta = 1 * thetaStep;
    // makeCapSlice(currentTheta, nextTheta);
    // makeSlopeSlice(currentTheta, nextTheta);

    makeCylinder();

}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

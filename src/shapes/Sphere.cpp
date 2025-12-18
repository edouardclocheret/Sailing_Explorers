#include "Sphere.h"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = std::max(param1,2);
    m_param2 = std::max(param2,3);
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!


    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(bottomLeft));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(topRight));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // Task 6: create a single wedge of the sphere using the
    //         makeTile() function you implemented in Task 5
    // Note: think about how param 1 comes into play here!

    float phiStep = M_PI / m_param1;
    float radius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float phi1 = i * phiStep;
        float phi2 = (i + 1) * phiStep;

        glm::vec3 topLeft(
            radius * sin(phi1) * cos(currentTheta),
            radius * cos(phi1),
            -radius * sin(phi1) * sin(currentTheta));

        glm::vec3 topRight(
            radius * sin(phi1) * cos(nextTheta),
            radius * cos(phi1),
            -radius * sin(phi1) * sin(nextTheta));

        glm::vec3 bottomLeft(
            radius * sin(phi2) * cos(currentTheta),
            radius * cos(phi2),
            -radius * sin(phi2) * sin(currentTheta));

        glm::vec3 bottomRight(
            radius * sin(phi2) * cos(nextTheta),
            radius * cos(phi2),
            -radius * sin(phi2) * sin(nextTheta));
        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }

}

void Sphere::makeSphere() {
    // Task 7: create a full sphere using the makeWedge() function you
    //         implemented in Task 6
    // Note: think about how param 2 comes into play here!
    float thetaStep = 2*M_PI / m_param2;

    for (int j = 0; j < m_param2; j++) {
        float currentTheta = j * thetaStep;
        float nextTheta = (j + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    // Uncomment these lines to make a wedge for Task 6, then comment them out for Task 7:

    // float thetaStep = glm::radians(360.f / m_param2);
    // float currentTheta = 0 * thetaStep;
    // float nextTheta = 1 * thetaStep;
    // makeWedge(currentTheta, nextTheta);

    // Uncomment these lines to make sphere for Task 7:

    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

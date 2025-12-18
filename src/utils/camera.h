#pragma once

#include <glm/glm.hpp>

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
public:
    Camera(glm::vec4 pos4, glm::vec4 look4, glm::vec4 up4)
    {
        glm::vec3 posi = glm::vec3(pos4);
        glm::vec3 look = glm::vec3(look4);
        glm::vec3 up = glm::vec3(up4);

        glm::mat4 trans = glm::mat4(1.f,0.f,0.f,0.f,
                                    0.f,1.f,0.f,0.f,
                                    0.f,0.f,1.f,0.f,
                                    -posi[0], -posi[1], -posi[2], 1.f);
        glm::vec3 w = -normalize(look);
        glm::vec3 v = normalize(up - dot(up,w)*w);
        glm::vec3 u = cross(v,w);
        glm::mat4 rot = glm::mat4(u[0],v[0],w[0],0.f,
                                  u[1],v[1],w[1],0.f,
                                  u[2],v[2],w[2],0.f,
                                  0.f, 0.f, 0.f, 1.f);

        viewMat = rot*trans;
        inv_viewMat = glm::inverse(viewMat);
        pos.x = posi.x;
        pos.y = posi.y;
        pos.z = posi.z;
        pos[3] = 1;

        this->u = u;
        this->v = v;
    }

    Camera() : //default camera (has to be overwritten)
        inv_viewMat(1.0f)
    {}


    glm::mat4 inv_viewMat;
    glm::mat4 viewMat;
    float k;
    float w;
    float h;
    glm::vec3 u;
    glm::vec3 v;

private :

    float aspectRatio;
    float heightAngle;
    float focalLenght;
    glm::vec4 pos;


};



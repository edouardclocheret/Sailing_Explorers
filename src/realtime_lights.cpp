#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <glm/gtc/matrix_transform.hpp>



void Realtime::parse_lights (std::vector<SceneLightData>& lights){
    //empty the previous lights
    m_lights4GPU.clear();

    int i =0;
    for (SceneLightData light : lights){
        m_lights4GPU.push_back(Light4GPU());

        m_lights4GPU[i].valid = 1;
        m_lights4GPU[i].type = int(light.type);
        m_lights4GPU[i].color = light.color;
        m_lights4GPU[i].function = light.function;
        switch (light.type){
        case(LightType::LIGHT_POINT):
            m_lights4GPU[i].pos = light.pos;
            break;
        case(LightType::LIGHT_DIRECTIONAL):
            m_lights4GPU[i].dir = light.dir;
            break;
        case(LightType::LIGHT_SPOT):
            m_lights4GPU[i].pos = light.pos;
            m_lights4GPU[i].dir = light.dir;
            m_lights4GPU[i].penumbra = light.penumbra;
            m_lights4GPU[i].angle = light.angle;
            break;
        }
        i++;
    }
}

void Realtime::make_light_uniforms(GLuint m_shader){

    for (int j = 0; j < 8; j++) {
        auto sendInt = [&](std::string name, int value){
            GLint loc = glGetUniformLocation(m_shader, name.c_str());
            glUniform1i(loc, value);
        };

        auto sendFloat = [&](std::string name, float value){
            GLint loc = glGetUniformLocation(m_shader, name.c_str());
            glUniform1f(loc, value);
        };

        auto sendVec3 = [&](std::string name, const glm::vec3 &v){
            GLint loc = glGetUniformLocation(m_shader, name.c_str());
            glUniform3f(loc, v.x, v.y, v.z);
        };

        if (j>=m_lights4GPU.size()){
            sendInt("lights_8[" + std::to_string(j) + "].valid", 0);
            continue;
        }


        const Light4GPU &L = m_lights4GPU[j];

        sendInt("lights_8[" + std::to_string(j) + "].valid", L.valid);
        sendInt("lights_8[" + std::to_string(j) + "].type", L.type);
        sendVec3("lights_8[" + std::to_string(j) + "].color", L.color);
        sendVec3("lights_8[" + std::to_string(j) + "].function", L.function);
        sendVec3("lights_8[" + std::to_string(j) + "].pos", L.pos);
        sendVec3("lights_8[" + std::to_string(j) + "].dir", L.dir);
        sendFloat("lights_8[" + std::to_string(j) + "].penumbra", L.penumbra);
        sendFloat("lights_8[" + std::to_string(j) + "].angle", L.angle);
    }
}

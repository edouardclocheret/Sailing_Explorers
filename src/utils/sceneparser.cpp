#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>


void dfsPopulateRenderData(SceneNode* scene_root, std::vector<RenderShapeData>& shapes, std::vector<SceneLightData>& lights, glm::mat4 ctm){

    SceneTransformation* transformation;
    glm::mat4 T_mat;
    glm::mat4 S_mat;
    glm::mat4 R_mat;
    glm::mat4 mat;


    for (int i = 0; i < scene_root->transformations.size(); ++i){
        transformation = scene_root->transformations[i];

        switch(transformation->type){
        case (TransformationType::TRANSFORMATION_TRANSLATE):
            T_mat= glm::translate(transformation->translate);
            ctm = ctm * T_mat;
            break;
        case (TransformationType::TRANSFORMATION_SCALE):
            S_mat = glm::scale(transformation->scale);
            ctm = ctm * S_mat;
            break;
        case (TransformationType::TRANSFORMATION_ROTATE):
            R_mat = glm::rotate(transformation->angle, transformation->rotate);
            ctm = ctm* R_mat ;
            break;
        case (TransformationType::TRANSFORMATION_MATRIX):
            mat = transformation->matrix;
            ctm = ctm * mat ;
            break;
        default:
            break;
        }
    }


    RenderShapeData tmpShape;
    tmpShape.ctm = ctm; //same for all
    tmpShape.local_ctm = ctm;
    tmpShape.i_ctm = glm::inverse(ctm);
    tmpShape.is_land = scene_root->is_land;

    SceneLightData tmpLight;

    for (ScenePrimitive* primitive : scene_root->primitives){
        tmpShape.primitive = *primitive;

        shapes.push_back(tmpShape);
    }

    for (SceneLight* light : scene_root->lights) {
        tmpLight.id = light->id;
        tmpLight.type = light->type;
        tmpLight.color = light->color;
        tmpLight.function = light->function;

        switch (tmpLight.type) {
        case LightType::LIGHT_POINT:
            // position comes from translation of the CTM
            tmpLight.pos = ctm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            break;

        case LightType::LIGHT_DIRECTIONAL:
            // direction transformed by rotation only
            tmpLight.dir = glm::vec4(glm::normalize(glm::mat3(ctm) * glm::vec3(light->dir)), 0.0f);
            break;

        case LightType::LIGHT_SPOT:
            // both position (from translation) and rotated direction
            tmpLight.pos = ctm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            tmpLight.dir = glm::vec4(glm::normalize(glm::mat3(ctm) * glm::vec3(light->dir)), 0.0f);
            tmpLight.penumbra = light->penumbra;
            tmpLight.angle = light->angle;
            break;

        default:
            break;
        }

        lights.push_back(tmpLight);
    }

    for (SceneNode* child : scene_root->children){
        dfsPopulateRenderData(child, shapes, lights, ctm);
    }
}


bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // TODO: Use your Lab 5 code here
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();


    renderData.shapes.clear();
    renderData.lights.clear();
    SceneNode* scene_root = fileReader.getRootNode();
    glm::mat4 ctm = glm::mat4(1.0f);
    dfsPopulateRenderData(scene_root, renderData.shapes, renderData.lights, ctm);
    return true;
}

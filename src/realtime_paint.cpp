#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include "settings.h"


#include <glm/gtc/matrix_transform.hpp>


void Realtime::paintTexture(GLuint texture, bool post_process){

    glUseProgram(m_texture_shader);
    GLuint postPLoc = glGetUniformLocation(m_texture_shader, "post_process");
    glUniform1i(postPLoc, post_process);
    glBindVertexArray(m_fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);//6 vertices
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::paintDepth(){
    glUseProgram(m_depth_shader);

    GLuint nearLoc = glGetUniformLocation(m_depth_shader, "near");
    GLuint farLoc  = glGetUniformLocation(m_depth_shader, "far");
    glUniform1f(nearLoc, settings.nearPlane);
    glUniform1f(farLoc, settings.farPlane);


    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fbo_scene_depth);
    glUniform1i(glGetUniformLocation(m_depth_shader, "depth_sampler"), 1);

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6); //changed 3 to 6 for 2 triangles
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::paintVel(bool show_in_x){
    glUseProgram(m_velocity_shader);

    GLuint inxLoc = glGetUniformLocation(m_velocity_shader, "is_velx");
    glUniform1i(inxLoc, show_in_x);
    m_vpmat = m_proj *m_view;
    GLuint vpimatLoc = glGetUniformLocation(m_velocity_shader, "ViewProjectionMatrix");
    glUniformMatrix4fv(vpimatLoc,1,GL_FALSE,&m_vpmat[0][0]);
    GLuint prev_vpmatLoc = glGetUniformLocation(m_velocity_shader, "previousViewProjectionMatrix");
    glUniformMatrix4fv(prev_vpmatLoc,1,GL_FALSE,&m_previous_vpmat[0][0]);
    m_previous_vpmat = m_vpmat;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fbo_scene_depth);
    glUniform1i(glGetUniformLocation(m_velocity_shader, "depth_sampler"), 1);

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6); //changed 3 to 6 for 2 triangles
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::paintBlurredScene(){
    glUseProgram(m_motion_blur_shader);
    m_vpmat = m_proj *m_view;
    GLuint vpimatLoc = glGetUniformLocation(m_motion_blur_shader, "ViewProjectionMatrix");
    glUniformMatrix4fv(vpimatLoc,1,GL_FALSE,&m_vpmat[0][0]);
    GLuint prev_vpmatLoc = glGetUniformLocation(m_motion_blur_shader, "previousViewProjectionMatrix");
    glUniformMatrix4fv(prev_vpmatLoc,1,GL_FALSE,&m_previous_vpmat[0][0]);
    m_previous_vpmat = m_vpmat;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_scene_texture);
    glUniform1i(glGetUniformLocation(m_motion_blur_shader, "scene_sampler"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fbo_scene_depth);
    glUniform1i(glGetUniformLocation(m_motion_blur_shader, "depth_sampler"), 1);

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6); //changed 3 to 6 for 2 triangles
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void Realtime::paintGeometry(){
    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind Sphere Vertex Data


    paintFloor();

    glBindVertexArray(m_vao);

    // Task 2: activate the shader program by calling glUseProgram with `m_shader`
    glUseProgram(m_shader);

    // Task 6: pass in m_model as a uniform into the shader program

    // Task 7: pass in m_view and m_proj

    GLint viewLocation = glGetUniformLocation(m_shader, "view_mat");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &m_view[0][0]);
    GLint projLocation = glGetUniformLocation(m_shader, "proj_mat");
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &m_proj[0][0]);

    //global ka kd ks

    // Task 12: pass m_ka into the fragment shader as a uniform
    GLint kaLocation = glGetUniformLocation(m_shader, "ka");
    glUniform1f(kaLocation, m_renderData.globalData.ka);

    // Task 13: pass light position and m_kd into the fragment shader as a uniform
    GLint kdLocation = glGetUniformLocation(m_shader, "kd");
    glUniform1f(kdLocation, m_renderData.globalData.kd);

    //create a struct for 8 lights

    make_light_uniforms(m_shader);

    GLint camLocation = glGetUniformLocation(m_shader, "cam_pos");
    glm::vec4 cam_position = m_renderData.cameraData.pos;
    glUniform4fv(camLocation, 1, &cam_position[0]);


    // Task 14: pass shininess, m_ks, and world-space camera position
    GLint ksLocation = glGetUniformLocation(m_shader, "ks");
    glUniform1f(ksLocation, m_renderData.globalData.ks);

    // Upload fog uniforms if present in shader
    applyFogUniforms(m_shader);


    int index =0;
    int offset=0;
    for (const RenderShapeData &shape : m_renderData.shapes){
        m_model = shape.ctm;
        m_normal_mat = glm::inverse(glm::transpose(glm::mat3(m_model)));

        GLint modelLocation = glGetUniformLocation(m_shader, "model_mat");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &m_model[0][0]);

        GLint normMatLocation = glGetUniformLocation(m_shader,"normal_mat");
        glUniformMatrix3fv(normMatLocation, 1, GL_FALSE, &m_normal_mat[0][0]);

        GLint cAmbLocation = glGetUniformLocation(m_shader, "cAmbient");
        glUniform3fv(cAmbLocation, 1, &shape.primitive.material.cAmbient[0]);

        GLint cDifLocation = glGetUniformLocation(m_shader, "cDiffuse");
        glUniform3fv(cDifLocation, 1, &shape.primitive.material.cDiffuse[0]);

        GLint cSpeLocation = glGetUniformLocation(m_shader, "cSpecular");
        glUniform3fv(cSpeLocation, 1, &shape.primitive.material.cSpecular[0]);

        GLint nLocation = glGetUniformLocation(m_shader, "n");
        glUniform1f(nLocation, shape.primitive.material.shininess);

        // Draw Command
        int verts = m_sizes_v_m_shapesData[index] / 6;
        glDrawArrays(GL_TRIANGLES, offset, verts);
        index++;
        offset+= verts;
    }

    // Render L-Systems if enabled
    if (settings.enableLSystem && m_lsystemRenderer.hasPlants()) {
        glUseProgram(m_shader);
        GLint modelLocation = glGetUniformLocation(m_shader, "model_mat");
        GLint normMatLocation = glGetUniformLocation(m_shader, "normal_mat");
        glm::vec3 tint(1.0f);
        m_lsystemRenderer.render(m_shader, modelLocation, normMatLocation, tint, tint);
        glUseProgram(0);
    }

    // Unbind Vertex Array
    glBindVertexArray(0);

    // Task 3: deactivate the shader program by passing 0 into glUseProgram
    glUseProgram(0);

    renderInstancedForest();
    renderTreasure();

}


void Realtime::paintFloor() {

    glUseProgram(m_sea_shader);

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(m_sea_shader, "model"),
                       1, GL_FALSE, &model[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(m_sea_shader, "view"),
                       1, GL_FALSE, &m_view[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(m_sea_shader, "projection"),
                       1, GL_FALSE, &m_proj[0][0]);

    glm::mat3 normalMat = glm::inverse(glm::transpose(glm::mat3(model)));
    glUniformMatrix3fv(glGetUniformLocation(m_sea_shader, "normal_mat"),
                       1, GL_FALSE, &normalMat[0][0]);

    make_light_uniforms(m_sea_shader);

    glUniform1f(glGetUniformLocation(m_sea_shader, "ka"),
                m_renderData.globalData.ka);
    glUniform1f(glGetUniformLocation(m_sea_shader, "kd"),
                m_renderData.globalData.kd);
    glUniform1f(glGetUniformLocation(m_sea_shader, "ks"),
                m_renderData.globalData.ks);

    float time = m_cumulativeTimer.elapsed()*0.00004; //adapting the scroll speed
    glUniform1f(glGetUniformLocation(m_sea_shader, "u_time"), time);

    glm::vec3 cAmbient(0.0, 0.56,1.0);
    glm::vec3 cDiffuse(0.0, 0.56,1.0);
    glm::vec3 cSpecular(0.0, 0.56,1.0);
    float shininess = 5.0;
    glUniform3fv(glGetUniformLocation(m_sea_shader, "cAmbient"), 1, &cAmbient[0]);
    glUniform3fv(glGetUniformLocation(m_sea_shader, "cDiffuse"), 1, &cDiffuse[0]);
    glUniform3fv(glGetUniformLocation(m_sea_shader, "cSpecular"), 1, &cSpecular[0]);
    glUniform1f(glGetUniformLocation(m_sea_shader, "n"), shininess);

    glm::vec4 camPos = m_renderData.cameraData.pos;
    glUniform4fv(glGetUniformLocation(m_sea_shader, "cam_pos"),
                 1, &camPos[0]);

    glBindVertexArray(m_floorVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,m_floorTexture);
    GLuint samplerLoc = glGetUniformLocation(m_sea_shader, "texture_sampler");
    glUniform1i(samplerLoc, 0);
    applyFogUniforms(m_sea_shader);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
}


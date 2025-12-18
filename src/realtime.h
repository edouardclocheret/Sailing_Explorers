#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <utility>
#include <random>
#include <QString>
#include <QObject>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "utils/sceneparser.h"
#include "utils/camera.h"
#include "render/postprocessor.h"
#include "render/instancedtrees.h"
#include "lsystem/lsystemrenderer.h"

struct Light4GPU
{
    int valid = 0;
    int type; //0point 1dir 2spot
    glm::vec3 color;
    glm::vec3 function;
    glm::vec3 pos; //ctm already applied
    glm::vec3 dir; //ctm already applied
    float penumbra;
    float angle;
};


class Realtime : public QOpenGLWidget
{
    Q_OBJECT
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);
    void resetBoatAndCamera();

signals:
    void treasureCollected(int newScore);
    void treasureDirectionChanged(float angleDegrees);
protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames
    QElapsedTimer m_cumulativeTimer;

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;

    GLuint m_defaultFBO;
    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;

    RenderData m_renderData;
    SceneCameraData m_initialCameraData;
    Camera m_camera; //has the attribute m_camera.inv_view
    glm::mat4 m_model = glm::mat4(1);
    glm::mat4 m_view  = glm::mat4(1);
    glm::mat4 m_proj  = glm::mat4(1);

    glm::mat4 m_vpmat = glm::mat4(1); //projection view matrix
    glm::mat4 m_previous_vpmat = glm::mat4(1);

    void updateProjMat();
    void updateViewMat();

    GLuint m_shader;     // Stores id of shader program
    GLuint m_depth_shader;
    GLuint m_velocity_shader;
    GLuint m_motion_blur_shader;

    GLuint m_sea_shader;
    GLuint m_instancedTreeShader = 0;

    GLuint m_vbo=0; // Stores id of vbo
    GLuint m_vao =0; // Stores id of vao

    GLuint m_floorVAO = 0;
    GLuint m_floorVBO = 0;
    GLuint m_floorTexture = 0;

    void initializeFloor();
    void paintFloor();

    //optimization
    glm::mat3 m_normal_mat;

    std::vector<float> generateShapeData(RenderShapeData& shape, GLuint param1, GLuint param2);
    std::vector<float> m_shapesData;
    std::vector<int> m_sizes_v_m_shapesData;
    std::vector<Light4GPU> m_lights4GPU;
    void parse_lights (std::vector<SceneLightData>& lights);
    void make_light_uniforms(GLuint m_shader);

    std::vector<GLfloat> generateFloorData(float size);

    void rebuildGeometry();
    bool m_glReady = false; //cannot rebuild geometry before initialize gl ran

    void translateCamera(glm::vec3 dir);
    void rotateCamera(float angleRad, glm::vec3 axis);
    glm::vec3 rotateVector(glm::vec3 vec, float angleRad, glm::vec3 axis);
    void updateBoatShapes();

    // QPoint m_prevMousePos;
    // float  m_angleX;
    // float  m_angleY;
    // float  m_zoom;

    GLuint m_texture_shader;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    GLuint m_fbo_scene;
    GLuint m_fbo_scene_texture;
    GLuint m_fbo_scene_depth;
    void makeSceneFBO();

    GLuint m_fbo_depth;
    GLuint m_fbo_depth_texture;
    void makeDepthFBO();

    GLuint m_fbo_vel;
    GLuint m_fbo_vel_texture; //color texture has two channels, first for x, second for y
    void makeVelFBO();



    void initialize_textures();
    void makeFBOs();


    void paintGeometry();//default scene
    void paintTexture(GLuint texture, bool post_process);
    void paintDepth();
    void paintVel(bool show_in_x);
    void paintBlurredScene();
    void applyFogUniforms(GLuint program) const;

    glm::mat4 m_boatCTM = glm::mat4(1.0f);

    // Post-processing (DOF)
    PostProcessor m_postProcessor;

    // L-System renderer
    LSystemRenderer m_lsystemRenderer;
    InstancedTreeRenderer m_instancedTreeRenderer;

    struct AABB {
        glm::vec3 min;
        glm::vec3 max;
    };

    std::vector<AABB> m_landAABBs;
    std::vector<int> m_boatShapeIndices;
    float m_collisionMargin = 0.3f;

    void rebuildCollisionCaches();
    AABB computeAABB(const glm::mat4 &ctm, PrimitiveType type) const;
    AABB transformBounds(const glm::vec3 &minCorner, const glm::vec3 &maxCorner, const glm::mat4 &ctm) const;
    std::pair<glm::vec3, glm::vec3> primitiveBounds(PrimitiveType type) const;
    AABB shrinkAABB(const AABB &box, float margin) const;
    bool aabbIntersect(const AABB &a, const AABB &b) const;
    bool wouldCollide(const glm::mat4 &candidateBoatCTM) const;

    // Treasure helpers
    void initializeTreasureResources();
    void loadTreasureTexture();
    QString resolveTreasureTexturePath() const;
    void renderTreasure();
    void renderInstancedForest();
    void spawnTreasure();
    bool isWaterPosition(const glm::vec3 &pos) const;
    bool isWithinViewFrustum(const glm::vec3 &pos) const;
    bool boatCollidesTreasure() const;
    void updateTreasureState();
    AABB treasureAABB() const;
    void updateTreasureDirection();
    void populateFoliageOnLand();
    void refreshFoliage(bool force = false);
    void populateInstancedForest();
    void refreshInstancedForest(bool force = false);
    void generateProceduralLand();
    void appendLandPrimitive(const glm::vec3 &center,
                             const glm::vec3 &scale,
                             float yawRadians,
                             const glm::vec4 &diffuse,
                             PrimitiveType primitive,
                             float shininess = 6.0f) ;
    SceneMaterial makeLandMaterial(const glm::vec4 &diffuse, float shininess) const;

    GLuint m_treasureVAO = 0;
    GLuint m_treasureVBO = 0;
    GLuint m_treasureTexture = 0;
    GLuint m_treasureShader = 0;
    bool m_treasureTextureReady = false;
    bool m_treasureActive = false;
    glm::vec3 m_treasurePos = glm::vec3(0.0f);
    float m_treasureHalfWidth = 1.5f;
    float m_treasureHalfHeight = 1.1f;
    float m_treasureBaseHeight = 0.15f;
    float m_treasureMinDistance = 22.0f;
    float m_treasureMaxDistance = 44.0f;
    std::mt19937 m_rng;
    QString m_treasureTexturePath;
    int m_treasureScore = 0;
    float m_treasureArrowDegrees = 0.f;
    bool m_prevLSystemEnabled = false;
    int m_prevLSystemType = -1;
    int m_prevLSystemIterations = -1;
    float m_prevLSystemScale = -1.0f;
    bool m_prevInstancedForestEnabled = false;
    std::vector<glm::mat4> m_instancedTreeTransforms;

};

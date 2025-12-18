#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <iostream>
#include <algorithm>
#include <limits>
#include <array>
#include <cmath>
#include <vector>
#include "settings.h"
#include "src/shaderloader.h"
#include "lsystem/lsystem.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>


// ================== Rendering the Scene!

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent), m_rng(std::random_device{}())
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this

}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);

    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);

    glDeleteProgram(m_shader);
    glDeleteProgram(m_texture_shader);
    glDeleteProgram(m_depth_shader);
    glDeleteProgram(m_velocity_shader);
    glDeleteProgram(m_motion_blur_shader);
    glDeleteProgram(m_treasureShader);
    glDeleteProgram(m_instancedTreeShader);

    glDeleteTextures(1, &m_fbo_scene_depth);
    glDeleteTextures(1, &m_fbo_scene_texture);
    glDeleteTextures(1, &m_fbo_depth_texture);
    glDeleteTextures(1, &m_fbo_vel_texture);

    glDeleteFramebuffers(1, &m_fbo_scene);
    glDeleteFramebuffers(1, &m_fbo_depth);
    glDeleteFramebuffers(1, &m_fbo_vel);

    if (m_treasureVAO) {
        glDeleteVertexArrays(1, &m_treasureVAO);
        m_treasureVAO = 0;
    }
    if (m_treasureVBO) {
        glDeleteBuffers(1, &m_treasureVBO);
        m_treasureVBO = 0;
    }
    if (m_treasureTexture) {
        glDeleteTextures(1, &m_treasureTexture);
        m_treasureTexture = 0;
    }

    this->doneCurrent();

    // Finish post-processor and L-System renderer
    m_postProcessor.finish();
    m_lsystemRenderer.finish();
    m_instancedTreeRenderer.finish();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    m_defaultFBO = 4;

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();
    m_cumulativeTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    // Set clear color to black
    glClearColor(0.522, 0.851, 1.0, 1.0);

    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert",
                                                 ":/resources/shaders/default.frag");
    m_instancedTreeShader = ShaderLoader::createShaderProgram(":/resources/shaders/instanced_tree.vert",
                                                             ":/resources/shaders/default.frag");

    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    m_depth_shader = ShaderLoader::createShaderProgram(":/resources/shaders/depth.vert",
                                                       ":/resources/shaders/depth.frag");

    m_velocity_shader = ShaderLoader::createShaderProgram(":/resources/shaders/velocity.vert",
                                                       ":/resources/shaders/velocity.frag");

    m_motion_blur_shader = ShaderLoader::createShaderProgram(":/resources/shaders/motion_blur.vert",
                                                             ":/resources/shaders/motion_blur.frag");
    m_sea_shader = ShaderLoader::createShaderProgram(":/resources/shaders/sea.vert",
                                                     ":/resources/shaders/sea.frag");
    m_treasureShader = ShaderLoader::createShaderProgram(":/resources/shaders/treasure.vert",
                                                        ":/resources/shaders/treasure.frag");
    rebuildGeometry();
    updateProjMat();
    updateViewMat();

    initializeFloor();
    initializeTreasureResources();
    loadTreasureTexture();

    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    // +//UV//
            -1.f,  1.f, 0.0f,
            0.f,1.f,
            -1.f, -1.f, 0.0f,
            0.f,0.f,
            1.f, -1.f, 0.0f,
            1.f, 0.f,

            -1.f,  1.f, 0.0f,
            0.f,1.f,
            1.f,  -1.f, 0.0f,
            1.f, 0.f,
            1.f, 1.f, 0.0f,
            1.f,1.f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*) (0* sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*) (3* sizeof(GLfloat)));


    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBOs();

    // Initialize post-processor and L-System renderer
    m_postProcessor.initialize();
    m_lsystemRenderer.initialize();
    m_instancedTreeRenderer.initialize();

    m_glReady = true;

    spawnTreasure();
    refreshFoliage(true);
    refreshInstancedForest(true);
}

void Realtime::paintGL() {
    //puts depth and the regular phonged geometry in the scene fbo
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_scene);
    glViewport(0,0,m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    paintGeometry();

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

    if (settings.show_depth) {
        // 1. Paint depth as colors in my depth buffer texture
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_depth);
        glViewport(0,0,m_fbo_width, m_fbo_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        paintDepth();

        glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

        // 2. Paint depth on the screen
        paintTexture(m_fbo_depth_texture, false);

    }
    else if (settings.show_velocity_x || settings.show_velocity_y){
        // 1. Paint in my velocity buffer as colors in a texture
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_vel);
        glViewport(0,0,m_fbo_width, m_fbo_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        paintVel(settings.show_velocity_x);

        glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

        // 2. Paint depth on the screen
        paintTexture(m_fbo_vel_texture, false);
    }
    else if (settings.enable_motion_blur){
        paintBlurredScene();
    }
    else{
        if (settings.enableDOF) {
            DOFParams params;
            params.enabled = true;
            params.focalDistance = settings.dofFocalDistance;
            params.aperture = settings.dofAperture;
            params.maxBlur = settings.dofMaxBlur;
            m_postProcessor.render(m_fbo_scene_texture,
                                   m_fbo_scene_depth,
                                   settings.nearPlane,
                                   settings.farPlane,
                                   glm::ivec2(m_fbo_width, m_fbo_height),
                                   settings.show_depth,
                                   params);
        } else {
            paintTexture(m_fbo_scene_texture, false);
        }
    }
}



void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    //glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glViewport(0, 0, w * m_devicePixelRatio, h * m_devicePixelRatio);
    // Students: anything requiring OpenGL calls when the program starts should be done here

    rebuildGeometry();
    updateProjMat();
    updateViewMat();
    //because uses width and height for the aspect ratio
}

void Realtime::sceneChanged() {
    m_lights4GPU.clear();
    if (!SceneParser::parse(settings.sceneFilePath, m_renderData)) {
        std::cerr << "Failed to parse scene file: " << settings.sceneFilePath << std::endl;
        return;
    }

    generateProceduralLand();

    m_boatCTM = glm::mat4(1.0f);
    m_initialCameraData = m_renderData.cameraData;

    rebuildGeometry();
    updateProjMat();
    updateViewMat();

    rebuildCollisionCaches();
    updateBoatShapes();
    spawnTreasure();
    refreshFoliage(true);
    refreshInstancedForest(true);


    // Ask Qt to repaint the OpenGL widget
    update();
}


void Realtime::settingsChanged() {
    m_lights4GPU.clear();

    rebuildGeometry();
    updateProjMat();
    updateViewMat();
    refreshFoliage();
    refreshInstancedForest();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::rebuildGeometry() {
    if (!m_glReady) return;

    m_shapesData.clear();
    m_sizes_v_m_shapesData.clear();
    parse_lights(m_renderData.lights);

    // Re-generate vertex data for every shape
    for (auto &shape : m_renderData.shapes) {
        std::vector<float> data =
            generateShapeData(shape,
                              settings.shapeParameter1,
                              settings.shapeParameter2);

        m_sizes_v_m_shapesData.push_back(data.size());
        m_shapesData.insert(m_shapesData.end(), data.begin(), data.end());
    }

    // Upload to GPU
    if (m_vao == 0) {
        glGenVertexArrays(1, &m_vao);
    }
    glBindVertexArray(m_vao);

    if (m_vbo == 0) {
        glGenBuffers(1, &m_vbo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 m_shapesData.size() * sizeof(GLfloat),
                 m_shapesData.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}


void Realtime::mouseMoveEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    // Camera rotation by mouse has been disabled intentionally. The previous
    // implementation is preserved below for reference and can be restored
    // later if interactive camera controls should return.
    /*
    if (!m_mouseDown) return;

    float sens = 0.01f;

    int posX = event->position().x();
    int posY = event->position().y();
    int deltaX = posX - m_prev_mouse_pos.x;
    int deltaY = posY - m_prev_mouse_pos.y;
    m_prev_mouse_pos = glm::vec2(posX, posY);


    glm::vec4 center(0.f); //orbit arount the origin = boat position

    glm::vec4 pos = m_renderData.cameraData.pos;
    glm::vec4 rel = pos - center;

    float yaw = deltaX * sens;
    glm::vec3 axisY(0.f, 1.f, 0.f);

    float c = cos(yaw), s = sin(yaw), t = 1 - c;
    glm::vec3 a = glm::normalize(axisY);

    glm::mat4 R_yaw(
        t*a.x*a.x + c,       t*a.x*a.y - s*a.z, t*a.x*a.z + s*a.y, 0,
        t*a.x*a.y + s*a.z,   t*a.y*a.y + c,     t*a.y*a.z - s*a.x, 0,
        t*a.x*a.z - s*a.y,   t*a.y*a.z + s*a.x, t*a.z*a.z + c,     0,
        0,                   0,                 0,                 1
        );

    rel = glm::vec4(R_yaw * rel);
    rotateCamera(yaw, axisY);

    float pitch = deltaY * sens;

    glm::vec3 look = glm::normalize(center - (rel + center));
    glm::vec3 up   = glm::normalize(m_renderData.cameraData.up);
    glm::vec3 right = glm::normalize(glm::cross(look, up));

    a = glm::normalize(right);
    c = cos(pitch); s = sin(pitch); t = 1 - c;

    glm::mat4 R_pitch(
        t*a.x*a.x + c,       t*a.x*a.y - s*a.z, t*a.x*a.z + s*a.y, 0,
        t*a.x*a.y + s*a.z,   t*a.y*a.y + c,     t*a.y*a.z - s*a.x, 0,
        t*a.x*a.z - s*a.y,   t*a.y*a.z + s*a.x, t*a.z*a.z + c,     0,
        0,                   0,                 0,                 1
        );

    rel = R_pitch * rel;

    rotateCamera(pitch, right);

    m_renderData.cameraData.pos = center + rel;
    m_renderData.cameraData.look =
        glm::normalize(center - m_renderData.cameraData.pos);

    update();
    */
}


//boat rotates around its center not center of the world
glm::mat4 rotateAroundLocalY(const glm::mat4 &m, float angle) {
    // Rotate around local Y axis
    glm::vec3 pos = glm::vec3(m[3]);       // save current position
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0,1,0));
    glm::mat4 result = rot * m;
    result[3] = glm::vec4(pos, 1.0f);      // restore position
    return result;
}

std::pair<glm::vec3, glm::vec3> Realtime::primitiveBounds(PrimitiveType type) const {
    switch (type) {
    case PrimitiveType::PRIMITIVE_CUBE:
    case PrimitiveType::PRIMITIVE_SPHERE:
    case PrimitiveType::PRIMITIVE_CYLINDER:
    case PrimitiveType::PRIMITIVE_CONE:
        return {glm::vec3(-0.5f), glm::vec3(0.5f)};
    case PrimitiveType::PRIMITIVE_MESH:
    default:
        return {glm::vec3(-0.5f), glm::vec3(0.5f)};
    }
}

Realtime::AABB Realtime::transformBounds(const glm::vec3 &minCorner, const glm::vec3 &maxCorner, const glm::mat4 &ctm) const {
    glm::vec3 newMin(std::numeric_limits<float>::max());
    glm::vec3 newMax(std::numeric_limits<float>::lowest());

    for (int i = 0; i < 8; ++i) {
        glm::vec3 corner(
            (i & 1) ? maxCorner.x : minCorner.x,
            (i & 2) ? maxCorner.y : minCorner.y,
            (i & 4) ? maxCorner.z : minCorner.z
        );
        glm::vec4 world = ctm * glm::vec4(corner, 1.0f);
        glm::vec3 p = glm::vec3(world);
        newMin = glm::min(newMin, p);
        newMax = glm::max(newMax, p);
    }

    return {newMin, newMax};
}

Realtime::AABB Realtime::computeAABB(const glm::mat4 &ctm, PrimitiveType type) const {
    auto bounds = primitiveBounds(type);
    return transformBounds(bounds.first, bounds.second, ctm);
}

Realtime::AABB Realtime::shrinkAABB(const AABB &box, float margin) const {
    if (margin <= 0.f) {
        return box;
    }
    glm::vec3 center = 0.5f * (box.min + box.max);
    glm::vec3 half = 0.5f * (box.max - box.min) - glm::vec3(margin);
    half = glm::max(half, glm::vec3(0.0f));
    return {center - half, center + half};
}

bool Realtime::aabbIntersect(const AABB &a, const AABB &b) const {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
            (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

void Realtime::rebuildCollisionCaches() {
    m_landAABBs.clear();
    m_boatShapeIndices.clear();

    for (size_t i = 0; i < m_renderData.shapes.size(); ++i) {
        const RenderShapeData &shape = m_renderData.shapes[i];
        if (shape.is_land) {
            AABB box = computeAABB(shape.ctm, shape.primitive.type);
            m_landAABBs.push_back(shrinkAABB(box, m_collisionMargin));
        } else {
            m_boatShapeIndices.push_back(static_cast<int>(i));
        }
    }
}

bool Realtime::wouldCollide(const glm::mat4 &candidateBoatCTM) const {
    if (m_landAABBs.empty()) {
        return false;
    }

    for (int idx : m_boatShapeIndices) {
        const RenderShapeData &shape = m_renderData.shapes[idx];
        glm::mat4 worldCTM = candidateBoatCTM * shape.local_ctm;
        AABB boatBox = computeAABB(worldCTM, shape.primitive.type);

        for (const AABB &landBox : m_landAABBs) {
            if (aabbIntersect(boatBox, landBox)) {
                return true;
            }
        }
    }

    return false;
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();


    glm::vec3 look_n = glm::normalize(m_renderData.cameraData.look);

    if (m_keyMap[Qt::Key_I] )
        translateCamera(glm::vec4(look_n * 5.f * deltaTime, 0.f));
    if (m_keyMap[Qt::Key_O])
        translateCamera(glm::vec4(-look_n * 5.f * deltaTime, 0.f));


    if (m_keyMap[Qt::Key_Left]|| m_keyMap[Qt::Key_Right]) {
        float yaw = (m_keyMap[Qt::Key_Left] ? +1.f : -1.f) * deltaTime * 3.5f;
        glm::mat4 candidate = rotateAroundLocalY(m_boatCTM, yaw);
        if (!wouldCollide(candidate)) {
            m_boatCTM = candidate;
        }
    }

    if (m_keyMap[Qt::Key_Space]) {
        float speed = 5.f * deltaTime;

        glm::vec3 forward = -glm::normalize(glm::vec3(m_boatCTM[2]));
        glm::vec3 delta = forward * speed;

        glm::mat4 candidate = glm::translate(glm::mat4(1.0f), delta) * m_boatCTM;

        if (!wouldCollide(candidate)) {
            // Translate boat
            m_boatCTM = candidate;

            // Move camera along with the boat
            translateCamera(glm::vec4(delta, 0.f));
        }
    }


    updateBoatShapes();


    update(); // trigger repaint

    updateTreasureState();
    updateTreasureDirection();
}

void Realtime::resetBoatAndCamera() {
    m_boatCTM = glm::mat4(1.0f);
    m_renderData.cameraData = m_initialCameraData;
    updateViewMat();
    updateBoatShapes();
    spawnTreasure();
    emit treasureCollected(m_treasureScore);
    update();
}

void Realtime::initializeTreasureResources() {
    glGenVertexArrays(1, &m_treasureVAO);
    glGenBuffers(1, &m_treasureVBO);

    glBindVertexArray(m_treasureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_treasureVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::loadTreasureTexture() {
    QString path = resolveTreasureTexturePath();
    if (path.isEmpty()) {
        std::cerr << "Treasure texture not found; skipping treasure rendering." << std::endl;
        m_treasureTextureReady = false;
        return;
    }

    QImage img(path);
    if (img.isNull()) {
        std::cerr << "Failed to load treasure texture from " << path.toStdString() << std::endl;
        m_treasureTextureReady = false;
        return;
    }

    img = img.convertToFormat(QImage::Format_RGBA8888);

    if (m_treasureTexture == 0) {
        glGenTextures(1, &m_treasureTexture);
    }

    glBindTexture(GL_TEXTURE_2D, m_treasureTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_treasureTextureReady = true;
    m_treasureTexturePath = path;
}

QString Realtime::resolveTreasureTexturePath() const {
    QString candidates[] = {
        QDir::currentPath() + QDir::separator() + QStringLiteral("scenefiles/realtime/sail_game/treasure.png"),
        QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("../scenefiles/realtime/sail_game/treasure.png"),
        QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("../../scenefiles/realtime/sail_game/treasure.png"),
        QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("../../../scenefiles/realtime/sail_game/treasure.png"),
        QStringLiteral("resources/textures/treasure.png")
    };

    for (const QString &cand : candidates) {
        QFile file(cand);
        if (file.exists()) {
            return QFileInfo(file).absoluteFilePath();
        }
    }
    return QString();
}

void Realtime::spawnTreasure() {
    if (!m_treasureTextureReady) {
        return;
    }

    glm::vec3 boatPos = glm::vec3(m_boatCTM * glm::vec4(0, 0, 0, 1));
    glm::vec3 forward = -glm::vec3(m_boatCTM[2]);
    if (glm::length(forward) < 1e-4f) {
        forward = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    forward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));

    std::uniform_real_distribution<float> distanceDist(m_treasureMinDistance, m_treasureMaxDistance);
    std::uniform_real_distribution<float> angleDist(-glm::pi<float>(), glm::pi<float>());

    glm::vec3 candidate = boatPos;
    const int maxAttempts = 80;
    bool placed = false;

    for (int i = 0; i < maxAttempts; ++i) {
        float distance = distanceDist(m_rng);
        float angle = angleDist(m_rng);
        glm::vec3 offset = glm::vec3(std::cos(angle), 0.0f, std::sin(angle)) * distance;
        candidate = boatPos + offset;
        candidate.y = m_treasureBaseHeight + m_treasureHalfHeight;
        if (isWaterPosition(candidate)) {
            placed = true;
            break;
        }
    }

    if (!placed) {
        candidate = boatPos + forward * m_treasureMinDistance;
        candidate.y = m_treasureBaseHeight + m_treasureHalfHeight;
    }

    m_treasurePos = candidate;
    m_treasureActive = true;
    updateTreasureDirection();
}

bool Realtime::isWaterPosition(const glm::vec3 &pos) const {
    for (const AABB &land : m_landAABBs) {
        if (pos.x >= land.min.x && pos.x <= land.max.x &&
            pos.z >= land.min.z && pos.z <= land.max.z) {
            return false;
        }
    }
    return true;
}

bool Realtime::isWithinViewFrustum(const glm::vec3 &pos) const {
    glm::vec4 clip = m_proj * m_view * glm::vec4(pos, 1.0f);
    if (std::abs(clip.w) < 1e-5f) {
        return false;
    }
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return ndc.x > -0.8f && ndc.x < 0.8f &&
           ndc.y > -0.8f && ndc.y < 0.8f &&
           ndc.z > 0.0f && ndc.z < 1.0f;
}

Realtime::AABB Realtime::treasureAABB() const {
    glm::vec3 half(m_treasureHalfWidth, m_treasureHalfHeight, m_treasureHalfWidth);
    return {m_treasurePos - half, m_treasurePos + half};
}

bool Realtime::boatCollidesTreasure() const {
    if (!m_treasureActive) {
        return false;
    }

    AABB treasureBox = treasureAABB();

    for (int idx : m_boatShapeIndices) {
        if (idx < 0 || idx >= static_cast<int>(m_renderData.shapes.size())) {
            continue;
        }
        const RenderShapeData &shape = m_renderData.shapes[idx];
        AABB boatBox = computeAABB(shape.ctm, shape.primitive.type);
        if (aabbIntersect(boatBox, treasureBox)) {
            return true;
        }
    }

    return false;
}

void Realtime::updateTreasureState() {
    if (!m_treasureActive || !m_treasureTextureReady) {
        return;
    }

    if (boatCollidesTreasure()) {
        m_treasureActive = false;
        m_treasureScore++;
        emit treasureCollected(m_treasureScore);
        spawnTreasure();
    }
}

void Realtime::updateTreasureDirection() {
    if (!m_treasureActive) {
        return;
    }

    glm::vec3 boatPos = glm::vec3(m_boatCTM * glm::vec4(0, 0, 0, 1));
    glm::vec3 toTreasure = glm::vec3(m_treasurePos) - boatPos;
    glm::vec3 flatToTreasure = glm::vec3(toTreasure.x, 0.0f, toTreasure.z);
    if (glm::length(flatToTreasure) < 1e-4f) {
        return;
    }
    flatToTreasure = glm::normalize(flatToTreasure);

    glm::vec3 boatForward = -glm::vec3(m_boatCTM[2]);
    if (glm::length(boatForward) < 1e-4f) {
        boatForward = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    glm::vec3 flatForward = glm::vec3(boatForward.x, 0.0f, boatForward.z);
    if (glm::length(flatForward) < 1e-4f) {
        flatForward = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    flatForward = glm::normalize(flatForward);

    glm::vec3 boatRight = glm::vec3(m_boatCTM[0]);
    if (glm::length(boatRight) < 1e-4f) {
        boatRight = glm::cross(flatForward, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if (glm::length(boatRight) < 1e-4f) {
        boatRight = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    glm::vec3 flatRight = glm::normalize(glm::vec3(boatRight.x, 0.0f, boatRight.z));

    float angleRad = atan2(glm::dot(flatToTreasure, flatRight), glm::dot(flatToTreasure, flatForward));
    float angleDeg = glm::degrees(angleRad);

    if (std::abs(angleDeg - m_treasureArrowDegrees) > 0.5f) {
        m_treasureArrowDegrees = angleDeg;
        emit treasureDirectionChanged(angleDeg);
    }
}

SceneMaterial Realtime::makeLandMaterial(const glm::vec4 &diffuse, float shininess) const {
    SceneMaterial mat{};
    glm::vec3 rgb = glm::vec3(diffuse);
    rgb = glm::clamp(rgb, glm::vec3(0.0f), glm::vec3(1.0f));
    mat.cDiffuse = glm::vec4(rgb, 1.0f);
    mat.cAmbient = glm::vec4(rgb * 0.55f, 1.0f);
    mat.cSpecular = glm::vec4(0.08f, 0.08f, 0.08f, 1.0f);
    mat.shininess = shininess;
    mat.cReflective = glm::vec4(0.0f);
    mat.cTransparent = glm::vec4(0.0f);
    mat.textureMap.clear();
    mat.blend = 0.0f;
    mat.cEmissive = glm::vec4(0.0f);
    mat.bumpMap.clear();
    return mat;
}

void Realtime::appendLandPrimitive(const glm::vec3 &center,
                                   const glm::vec3 &scale,
                                   float yawRadians,
                                   const glm::vec4 &diffuse,
                                   PrimitiveType primitive,
                                   float shininess) {
    RenderShapeData shape{};
    shape.primitive.type = primitive;
    shape.primitive.material = makeLandMaterial(diffuse, shininess);
    shape.primitive.meshfile.clear();

    glm::mat4 model(1.0f);
    model = glm::translate(model, center);
    model = glm::rotate(model, yawRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale);

    shape.ctm = model;
    shape.local_ctm = model;
    shape.i_ctm = glm::inverse(model);
    shape.is_land = true;

    m_renderData.shapes.push_back(shape);
}

void Realtime::generateProceduralLand() {
    if (m_renderData.shapes.empty()) {
        return;
    }

    std::uniform_int_distribution<int> clusterCountDist(6, 9);
    std::uniform_int_distribution<int> patchCountDist(4, 9);
    std::uniform_real_distribution<float> radialDist(15.0f, 65.0f);
    std::uniform_real_distribution<float> jitterDist(-8.5f, 8.5f);
    std::uniform_real_distribution<float> radiusDist(2.5f, 14.0f);
    std::uniform_real_distribution<float> baseHeightDist(0.25f, 1.4f);
    std::uniform_real_distribution<float> crownHeightDist(0.45f, 2.3f);
    std::uniform_real_distribution<float> tintDist(0.85f, 1.15f);
    std::uniform_real_distribution<float> sandTintDist(0.9f, 1.25f);
    std::uniform_real_distribution<float> rockTintDist(0.75f, 1.05f);
    std::uniform_real_distribution<float> yawDist(0.0f, glm::two_pi<float>());
    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> rippleDist(-0.4f, 0.4f);

    auto randomAnchor = [&]() {
        glm::vec2 anchor(0.0f);
        for (int attempt = 0; attempt < 12; ++attempt) {
            float theta = yawDist(m_rng);
            float dist = radialDist(m_rng);
            anchor = glm::vec2(std::cos(theta) * dist, std::sin(theta) * dist);
            if (glm::length(anchor) > 10.0f) {
                break;
            }
        }
        return anchor;
    };

    int clusterCount = clusterCountDist(m_rng);

    for (int cluster = 0; cluster < clusterCount; ++cluster) {
        glm::vec2 anchor = randomAnchor();
        float clusterYaw = yawDist(m_rng);
        float cosYaw = std::cos(clusterYaw);
        float sinYaw = std::sin(clusterYaw);
        int patchCount = patchCountDist(m_rng);

        for (int patch = 0; patch < patchCount; ++patch) {
            glm::vec2 jitter(jitterDist(m_rng), jitterDist(m_rng));
            glm::vec2 rotated(jitter.x * cosYaw - jitter.y * sinYaw,
                              jitter.x * sinYaw + jitter.y * cosYaw);
            glm::vec2 patchPos = anchor + rotated;
            if (glm::length(patchPos) < 9.0f) {
                continue;
            }

            float radius = radiusDist(m_rng) * (0.75f + 0.35f * chanceDist(m_rng));
            float baseHeight = baseHeightDist(m_rng) * (0.65f + 0.5f * chanceDist(m_rng));
            float soilHeight = baseHeight * (0.65f + 0.25f * chanceDist(m_rng));
            float canopyHeight = crownHeightDist(m_rng) * (0.6f + 0.4f * chanceDist(m_rng));
            float canopyRadius = radius * (0.8f + 0.4f * chanceDist(m_rng));

            glm::vec4 sandColor(0.78f, 0.69f, 0.46f, 1.0f);
            sandColor *= sandTintDist(m_rng);
            sandColor.w = 1.0f;

            glm::vec4 soilColor(0.47f, 0.34f, 0.20f, 1.0f);
            soilColor *= tintDist(m_rng);
            soilColor.w = 1.0f;

            glm::vec4 grassColor(0.19f, 0.45f, 0.18f, 1.0f);
            grassColor *= tintDist(m_rng);
            grassColor.w = 1.0f;

            glm::vec3 baseCenter(patchPos.x,
                                 -0.25f + baseHeight * 0.5f,
                                 patchPos.y);
            appendLandPrimitive(baseCenter,
                                glm::vec3(radius * 1.8f, baseHeight, radius * 1.8f),
                                yawDist(m_rng),
                                sandColor,
                                PrimitiveType::PRIMITIVE_CUBE,
                                3.5f);

            glm::vec3 soilCenter(patchPos.x,
                                 baseCenter.y + baseHeight * 0.5f,
                                 patchPos.y);
            appendLandPrimitive(soilCenter,
                                glm::vec3(radius * 1.25f, soilHeight, radius * 1.25f),
                                yawDist(m_rng),
                                soilColor,
                                PrimitiveType::PRIMITIVE_CYLINDER,
                                6.0f);

            glm::vec3 canopyCenter(patchPos.x + rippleDist(m_rng) * 1.5f,
                                   soilCenter.y + soilHeight * (0.4f + 0.35f * chanceDist(m_rng)),
                                   patchPos.y + rippleDist(m_rng) * 1.5f);
            float canopyScaleY = canopyHeight * (0.5f + 0.4f * chanceDist(m_rng));
            appendLandPrimitive(canopyCenter,
                                glm::vec3(canopyRadius, canopyScaleY, canopyRadius),
                                yawDist(m_rng),
                                grassColor,
                                PrimitiveType::PRIMITIVE_SPHERE,
                                2.8f);

            if (chanceDist(m_rng) < 0.35f) {
                glm::vec4 rockColor(0.30f, 0.32f, 0.34f, 1.0f);
                rockColor *= rockTintDist(m_rng);
                rockColor.w = 1.0f;
                glm::vec3 rockCenter(patchPos.x + jitterDist(m_rng) * 0.35f,
                                     soilCenter.y + soilHeight * (0.2f + 0.3f * chanceDist(m_rng)),
                                     patchPos.y + jitterDist(m_rng) * 0.35f);
                appendLandPrimitive(rockCenter,
                                    glm::vec3(radius * (0.2f + 0.35f * chanceDist(m_rng)), soilHeight * (0.3f + 0.45f * chanceDist(m_rng)), radius * (0.2f + 0.35f * chanceDist(m_rng))),
                                    yawDist(m_rng),
                                    rockColor,
                                    PrimitiveType::PRIMITIVE_CUBE,
                                    8.0f);
            }

            if (chanceDist(m_rng) < 0.28f) {
                glm::vec4 duneColor(0.73f, 0.66f, 0.48f, 1.0f);
                duneColor *= sandTintDist(m_rng);
                duneColor.w = 1.0f;
                float duneRadius = radius * (0.4f + 0.3f * chanceDist(m_rng));
                float duneHeight = baseHeight * (0.4f + 0.25f * chanceDist(m_rng));
                glm::vec3 duneCenter(patchPos.x + jitterDist(m_rng) * 0.5f,
                                     baseCenter.y + baseHeight * 0.4f,
                                     patchPos.y + jitterDist(m_rng) * 0.5f);
                appendLandPrimitive(duneCenter,
                                    glm::vec3(duneRadius, duneHeight, duneRadius),
                                    yawDist(m_rng),
                                    duneColor,
                                    PrimitiveType::PRIMITIVE_CYLINDER,
                                    2.0f);
            }
        }
    }
}

void Realtime::populateFoliageOnLand() {
    m_lsystemRenderer.clear();

    if (!m_glReady || !settings.enableLSystem || m_landAABBs.empty()) {
        m_prevLSystemEnabled = settings.enableLSystem;
        m_prevLSystemType = settings.lsystemType;
        m_prevLSystemIterations = settings.lsystemIterations;
        m_prevLSystemScale = settings.lsystemScale;
        return;
    }

    std::uniform_real_distribution<float> unitDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> scaleDist(1.1f, 2.0f);
    std::uniform_real_distribution<float> rotDist(0.0f, glm::two_pi<float>());
    std::uniform_real_distribution<float> colorDist(0.85f, 1.15f);
    std::uniform_real_distribution<float> heightDist(-0.05f, 0.18f);
    std::uniform_int_distribution<int> iterJitter(-1, 1);
    std::discrete_distribution<int> typeWeights({1, 4, 1, 1});

    const int globalPlantCap = 1100;
    int totalSpawned = 0;

    auto spawnPlant = [&](float x, float y, float z, bool prioritizeTrees, int &landCounter) {
        if (totalSpawned >= globalPlantCap) {
            return false;
        }

        float scale = settings.lsystemScale * scaleDist(m_rng);
        if (prioritizeTrees) {
            scale *= 1.15f;
        }

        int iterations = glm::clamp(settings.lsystemIterations + iterJitter(m_rng) + (prioritizeTrees ? 1 : 0), 3, 8);
        float yaw = rotDist(m_rng);

        LSystemRenderer::PlantType type = prioritizeTrees
            ? LSystemRenderer::PlantType::Tree
            : static_cast<LSystemRenderer::PlantType>(typeWeights(m_rng));

    glm::vec3 branchColor = glm::vec3(0.42f, 0.28f, 0.14f) * colorDist(m_rng);
    glm::vec3 leafColor = glm::vec3(0.24f, 0.65f, 0.28f) * colorDist(m_rng);
        branchColor = glm::clamp(branchColor, glm::vec3(0.08f), glm::vec3(1.0f));
        leafColor = glm::clamp(leafColor, glm::vec3(0.08f), glm::vec3(1.0f));

        m_lsystemRenderer.generatePlant(type,
                                        iterations,
                                        glm::vec3(x, y, z),
                                        scale,
                                        yaw,
                                        branchColor,
                                        leafColor);
        totalSpawned++;
        landCounter++;
        return true;
    };

    for (const AABB &land : m_landAABBs) {
        if (totalSpawned >= globalPlantCap) {
            break;
        }

        float width = land.max.x - land.min.x;
        float depth = land.max.z - land.min.z;
        float area = width * depth;
        if (area <= 0.1f) {
            continue;
        }

        int plantsThisLand = 0;
        const float targetSpacing = 3.2f;
        int cellsX = std::max(1, std::min(45, static_cast<int>(std::ceil(width / targetSpacing))));
        int cellsZ = std::max(1, std::min(45, static_cast<int>(std::ceil(depth / targetSpacing))));
        float cellWidth = width / static_cast<float>(cellsX);
        float cellDepth = depth / static_cast<float>(cellsZ);
    float fillProbability = std::clamp(0.6f + (area / 420.0f) + unitDist(m_rng) * 0.25f, 0.6f, 0.97f);
    int desiredMinPlants = std::max(3, static_cast<int>(area * 0.05f));

        for (int gx = 0; gx < cellsX && totalSpawned < globalPlantCap; ++gx) {
            for (int gz = 0; gz < cellsZ && totalSpawned < globalPlantCap; ++gz) {
                if (unitDist(m_rng) > fillProbability) {
                    continue;
                }

                float cellMinX = land.min.x + gx * cellWidth;
                float cellMaxX = cellMinX + cellWidth;
                float cellMinZ = land.min.z + gz * cellDepth;
                float cellMaxZ = cellMinZ + cellDepth;

                float x = cellMinX + unitDist(m_rng) * (cellMaxX - cellMinX);
                float z = cellMinZ + unitDist(m_rng) * (cellMaxZ - cellMinZ);
                float y = land.max.y + 0.07f + heightDist(m_rng);

                bool prioritizeTrees = unitDist(m_rng) < 0.65f;
                spawnPlant(x, y, z, prioritizeTrees, plantsThisLand);
            }
        }

        int scatterCount = std::clamp(static_cast<int>(area * 0.12f), 3, 30);
        for (int i = 0; i < scatterCount && totalSpawned < globalPlantCap; ++i) {
            float x = land.min.x + unitDist(m_rng) * width;
            float z = land.min.z + unitDist(m_rng) * depth;
            float y = land.max.y + 0.05f + heightDist(m_rng);
            spawnPlant(x, y, z, true, plantsThisLand);
        }

        if (plantsThisLand == 0 && totalSpawned < globalPlantCap) {
            glm::vec3 center = 0.5f * (land.min + land.max);
            float y = land.max.y + 0.08f + heightDist(m_rng);
            spawnPlant(center.x, y, center.z, true, plantsThisLand);
        }

        while (plantsThisLand < desiredMinPlants && totalSpawned < globalPlantCap) {
            float x = land.min.x + unitDist(m_rng) * width;
            float z = land.min.z + unitDist(m_rng) * depth;
            float y = land.max.y + 0.05f + heightDist(m_rng);
            spawnPlant(x, y, z, true, plantsThisLand);
        }
    }

    m_prevLSystemEnabled = true;
    m_prevLSystemType = settings.lsystemType;
    m_prevLSystemIterations = settings.lsystemIterations;
    m_prevLSystemScale = settings.lsystemScale;
}

void Realtime::refreshFoliage(bool force) {
    if (!settings.enableLSystem) {
        if (m_prevLSystemEnabled || force) {
            m_lsystemRenderer.clear();
        }
        m_prevLSystemEnabled = false;
        return;
    }

    bool needsRebuild = force ||
                        !m_prevLSystemEnabled ||
                        settings.lsystemType != m_prevLSystemType ||
                        settings.lsystemIterations != m_prevLSystemIterations ||
                        std::abs(settings.lsystemScale - m_prevLSystemScale) > 1e-3f;

    if (!needsRebuild && m_lsystemRenderer.hasPlants()) {
        return;
    }

    populateFoliageOnLand();
}

void Realtime::populateInstancedForest() {
    m_instancedTreeTransforms.clear();

    if (!m_glReady || !settings.enableInstancedForest || m_landAABBs.empty()) {
        m_instancedTreeRenderer.clearInstances();
        m_prevInstancedForestEnabled = settings.enableInstancedForest;
        return;
    }

    std::uniform_real_distribution<float> unitDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> offsetDist(-0.45f, 0.45f);
    std::uniform_real_distribution<float> scaleDist(0.7f, 1.9f);
    std::uniform_real_distribution<float> stretchDist(1.0f, 1.8f);
    std::uniform_real_distribution<float> heightDist(0.0f, 0.12f);
    std::uniform_real_distribution<float> rotDist(0.0f, glm::two_pi<float>());

    const size_t globalInstanceCap = 6000;
    const float targetSpacing = 2.4f;
    const float clearingRadius = 11.0f;

    for (const AABB &land : m_landAABBs) {
        float width = land.max.x - land.min.x;
        float depth = land.max.z - land.min.z;
        if (width <= 0.25f || depth <= 0.25f) {
            continue;
        }

        int cellsX = std::max(1, std::min(140, static_cast<int>(std::ceil(width / targetSpacing))));
        int cellsZ = std::max(1, std::min(140, static_cast<int>(std::ceil(depth / targetSpacing))));
        float cellWidth = width / static_cast<float>(cellsX);
        float cellDepth = depth / static_cast<float>(cellsZ);
        float fillProbability = std::clamp(0.65f + unitDist(m_rng) * 0.3f, 0.5f, 0.98f);

        for (int gx = 0; gx < cellsX && m_instancedTreeTransforms.size() < globalInstanceCap; ++gx) {
            for (int gz = 0; gz < cellsZ && m_instancedTreeTransforms.size() < globalInstanceCap; ++gz) {
                if (unitDist(m_rng) > fillProbability) {
                    continue;
                }

                float cellMinX = land.min.x + gx * cellWidth;
                float cellMinZ = land.min.z + gz * cellDepth;
                float x = cellMinX + unitDist(m_rng) * cellWidth + offsetDist(m_rng);
                float z = cellMinZ + unitDist(m_rng) * cellDepth + offsetDist(m_rng);
                if (glm::length(glm::vec2(x, z)) < clearingRadius) {
                    continue;
                }
                float y = land.max.y + heightDist(m_rng);

                float yaw = rotDist(m_rng);
                float scale = scaleDist(m_rng);
                float stretch = stretchDist(m_rng);

                glm::mat4 model(1.0f);
                model = glm::translate(model, glm::vec3(x, y, z));
                model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(scale, scale * stretch, scale));
                m_instancedTreeTransforms.push_back(model);
            }
        }

        int scatterCount = std::clamp(static_cast<int>(width * depth * 0.18f), 6, 240);
        for (int i = 0; i < scatterCount && m_instancedTreeTransforms.size() < globalInstanceCap; ++i) {
            float x = land.min.x + unitDist(m_rng) * width;
            float z = land.min.z + unitDist(m_rng) * depth;
            if (glm::length(glm::vec2(x, z)) < clearingRadius) {
                continue;
            }
            float y = land.max.y + heightDist(m_rng);
            float yaw = rotDist(m_rng);
            float scale = scaleDist(m_rng) * 1.05f;
            float stretch = stretchDist(m_rng) * 1.1f;

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(scale, scale * stretch, scale));
            m_instancedTreeTransforms.push_back(model);
        }

        if (m_instancedTreeTransforms.size() >= globalInstanceCap) {
            break;
        }
    }

    if (m_instancedTreeTransforms.empty()) {
        m_instancedTreeRenderer.clearInstances();
    } else {
        m_instancedTreeRenderer.updateInstances(m_instancedTreeTransforms);
    }

    m_prevInstancedForestEnabled = true;
}

void Realtime::refreshInstancedForest(bool force) {
    if (!m_glReady) {
        return;
    }

    if (!settings.enableInstancedForest) {
        if (m_prevInstancedForestEnabled || force) {
            m_instancedTreeTransforms.clear();
            m_instancedTreeRenderer.clearInstances();
        }
        m_prevInstancedForestEnabled = false;
        return;
    }

    bool needsRebuild = force || !m_prevInstancedForestEnabled || m_instancedTreeTransforms.empty();
    if (!needsRebuild) {
        return;
    }

    populateInstancedForest();
}

void Realtime::applyFogUniforms(GLuint program) const {
    if (program == 0) {
        return;
    }

    GLint fogEnabledLoc = glGetUniformLocation(program, "uFogEnabled");
    if (fogEnabledLoc == -1) {
        return;
    }

    glUniform1i(fogEnabledLoc, settings.enableFog ? 1 : 0);

    GLint fogTypeLoc = glGetUniformLocation(program, "uFogType");
    if (fogTypeLoc != -1) {
        glUniform1i(fogTypeLoc, settings.fogType);
    }

    GLint fogDensityLoc = glGetUniformLocation(program, "uFogDensity");
    if (fogDensityLoc != -1) {
        glUniform1f(fogDensityLoc, settings.fogDensity);
    }

    GLint fogStartLoc = glGetUniformLocation(program, "uFogStart");
    if (fogStartLoc != -1) {
        glUniform1f(fogStartLoc, settings.fogStart);
    }

    GLint fogEndLoc = glGetUniformLocation(program, "uFogEnd");
    if (fogEndLoc != -1) {
        glUniform1f(fogEndLoc, settings.fogEnd);
    }

    glm::vec3 fogColor(settings.fogColorR, settings.fogColorG, settings.fogColorB);
    GLint fogColorLoc = glGetUniformLocation(program, "uFogColor");
    if (fogColorLoc != -1) {
        glUniform3fv(fogColorLoc, 1, &fogColor[0]);
    }

    glm::vec3 camPos = glm::vec3(m_renderData.cameraData.pos);
    GLint fogCamLoc = glGetUniformLocation(program, "uFogCameraPos");
    if (fogCamLoc != -1) {
        glUniform3fv(fogCamLoc, 1, &camPos[0]);
    }
}

void Realtime::renderTreasure() {
    if (!m_treasureActive || !m_treasureTextureReady || m_treasureShader == 0) {
        return;
    }

    glm::vec3 cameraPos = glm::vec3(m_renderData.cameraData.pos);
    float timeSeconds = static_cast<float>(m_cumulativeTimer.elapsed()) * 0.001f;
    float bobOffset = std::sin(timeSeconds * 2.0f) * 0.2f;
    glm::vec3 visualCenter = m_treasurePos + glm::vec3(0.0f, bobOffset, 0.0f);

    glm::vec3 forward = cameraPos - visualCenter;
    forward.y = 0.0f;
    if (glm::length(forward) < 1e-4f) {
        forward = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    forward = glm::normalize(forward);

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(up, forward);
    if (glm::length(right) < 1e-4f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    right = glm::normalize(right);

    glm::vec3 rightOffset = right * m_treasureHalfWidth;
    glm::vec3 upOffset = up * m_treasureHalfHeight;

    glm::vec3 bottomCenter = visualCenter - upOffset;
    glm::vec3 topCenter = visualCenter + upOffset;

    glm::vec3 bl = bottomCenter - rightOffset;
    glm::vec3 br = bottomCenter + rightOffset;
    glm::vec3 tl = topCenter - rightOffset;
    glm::vec3 tr = topCenter + rightOffset;

    std::array<float, 30> vertices = {
        bl.x, bl.y, bl.z, 0.0f, 1.0f,
        br.x, br.y, br.z, 1.0f, 1.0f,
        tr.x, tr.y, tr.z, 1.0f, 0.0f,

        bl.x, bl.y, bl.z, 0.0f, 1.0f,
        tr.x, tr.y, tr.z, 1.0f, 0.0f,
        tl.x, tl.y, tl.z, 0.0f, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_treasureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLboolean cullEnabled = glIsEnabled(GL_CULL_FACE);
    if (cullEnabled) {
        glDisable(GL_CULL_FACE);
    }

    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint prevSrcRGB, prevDstRGB, prevSrcAlpha, prevDstAlpha;
    glGetIntegerv(GL_BLEND_SRC_RGB, &prevSrcRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &prevDstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevSrcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &prevDstAlpha);
    if (!blendEnabled) {
        glEnable(GL_BLEND);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_treasureShader);
    glUniformMatrix4fv(glGetUniformLocation(m_treasureShader, "uView"), 1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_treasureShader, "uProj"), 1, GL_FALSE, &m_proj[0][0]);
    applyFogUniforms(m_treasureShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_treasureTexture);
    glUniform1i(glGetUniformLocation(m_treasureShader, "treasureTex"), 0);

    glBindVertexArray(m_treasureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_treasureVBO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    if (cullEnabled) {
        glEnable(GL_CULL_FACE);
    }
    glBlendFuncSeparate(prevSrcRGB, prevDstRGB, prevSrcAlpha, prevDstAlpha);
    if (!blendEnabled) {
        glDisable(GL_BLEND);
    }
}

void Realtime::renderInstancedForest() {
    if (!settings.enableInstancedForest || m_instancedTreeShader == 0 ||
        m_instancedTreeRenderer.getInstanceCount() == 0) {
        return;
    }

    glUseProgram(m_instancedTreeShader);

    GLboolean cullEnabled = glIsEnabled(GL_CULL_FACE);
    if (cullEnabled) {
        glDisable(GL_CULL_FACE);
    }

    GLint viewLoc = glGetUniformLocation(m_instancedTreeShader, "view_mat");
    GLint projLoc = glGetUniformLocation(m_instancedTreeShader, "proj_mat");
    if (viewLoc != -1) {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &m_view[0][0]);
    }
    if (projLoc != -1) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &m_proj[0][0]);
    }

    make_light_uniforms(m_instancedTreeShader);

    GLint camLoc = glGetUniformLocation(m_instancedTreeShader, "cam_pos");
    if (camLoc != -1) {
        glm::vec4 camPos = m_renderData.cameraData.pos;
        glUniform4fv(camLoc, 1, &camPos[0]);
    }

    glUniform1f(glGetUniformLocation(m_instancedTreeShader, "ka"), m_renderData.globalData.ka);
    glUniform1f(glGetUniformLocation(m_instancedTreeShader, "kd"), m_renderData.globalData.kd);
    glUniform1f(glGetUniformLocation(m_instancedTreeShader, "ks"), m_renderData.globalData.ks);

    applyFogUniforms(m_instancedTreeShader);

    auto setMaterial = [&](const glm::vec3 &ambient,
                           const glm::vec3 &diffuse,
                           const glm::vec3 &specular,
                           float shininess) {
        GLint cAmbLoc = glGetUniformLocation(m_instancedTreeShader, "cAmbient");
        GLint cDifLoc = glGetUniformLocation(m_instancedTreeShader, "cDiffuse");
        GLint cSpeLoc = glGetUniformLocation(m_instancedTreeShader, "cSpecular");
        GLint nLoc = glGetUniformLocation(m_instancedTreeShader, "n");
        if (cAmbLoc != -1) glUniform3fv(cAmbLoc, 1, &ambient[0]);
        if (cDifLoc != -1) glUniform3fv(cDifLoc, 1, &diffuse[0]);
        if (cSpeLoc != -1) glUniform3fv(cSpeLoc, 1, &specular[0]);
        if (nLoc != -1) glUniform1f(nLoc, shininess);
    };

    glm::vec3 trunkAmbient(0.24f, 0.17f, 0.11f);
    glm::vec3 trunkDiffuse(0.34f, 0.22f, 0.13f);
    glm::vec3 trunkSpecular(0.08f, 0.08f, 0.08f);
    setMaterial(trunkAmbient, trunkDiffuse, trunkSpecular, 24.0f);
    m_instancedTreeRenderer.drawTrunks();

    glm::vec3 leafAmbient(0.12f, 0.24f, 0.13f);
    glm::vec3 leafDiffuse(0.22f, 0.56f, 0.19f);
    glm::vec3 leafSpecular(0.10f, 0.18f, 0.10f);
    setMaterial(leafAmbient, leafDiffuse, leafSpecular, 12.0f);
    m_instancedTreeRenderer.drawCanopies();

    glBindVertexArray(0);
    if (cullEnabled) {
        glEnable(GL_CULL_FACE);
    }
    glUseProgram(0);
}


// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}


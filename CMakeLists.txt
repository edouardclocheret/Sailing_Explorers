cmake_minimum_required(VERSION 3.16)

# Sets project name
project(saling_explorers LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Xml)

# Allows you to include files from within those directories
include_directories(src)

# GLM
add_subdirectory(glm)

# GLEW (static)
add_library(StaticGLEW STATIC glew/src/glew.c)
target_include_directories(StaticGLEW PUBLIC glew/include)

# Main executable sources + headers
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp

  src/render/postprocessor.cpp
  src/render/postprocessor.h
  src/render/instancedtrees.cpp
  src/render/instancedtrees.h

  src/lsystem/lsystem.cpp
  src/lsystem/lsystem.h
  src/lsystem/lsystemrenderer.cpp
  src/lsystem/lsystemrenderer.h

    src/mainwindow.h
    src/realtime.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp
    src/utils/camera.h
    src/utils/camera.cpp

    src/shapes/Cone.cpp src/shapes/Cone.h
    src/shapes/Cube.cpp src/shapes/Cube.h
    src/shapes/Tet.cpp src/shapes/Tet.h
    src/shapes/Cylinder.cpp src/shapes/Cylinder.h
    src/shapes/Cylinder.cpp src/shapes/Cylinder.h
    src/shapes/Cylinder.cpp src/shapes/Cylinder.h
    src/shapes/Sphere.cpp src/shapes/Sphere.h
    src/shapes/shape.h

    src/shaderloader.h
    src/shapes/shape.h
    src/realtime_paint.cpp
    src/realtime_geometry.cpp
    src/realtime_lights.cpp
    src/realtime_fbo.cpp
)

# Link Qt + OpenGL + GLEW
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Qt resources (shaders, etc)
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX "/"
    FILES
        resources/shaders/default.frag
        resources/shaders/default.vert
  resources/shaders/instanced_tree.vert
        resources/shaders/texture.frag
        resources/shaders/texture.vert
        resources/shaders/sea.frag
        resources/shaders/sea.vert
        resources/shaders/velocity.frag
        resources/shaders/velocity.vert
        resources/shaders/motion_blur.frag
        resources/shaders/motion_blur.vert
  resources/shaders/treasure.frag
  resources/shaders/treasure.vert
  resources/shaders/postprocess.frag
  resources/shaders/postprocess.vert
        resources/shaders/depth.frag
        resources/shaders/depth.vert
        # resources/textures/wave-normal-texture.jpg
        # resources/textures/water-normal-texture.jpg
        # resources/textures/water-normal-texture-2.jpg
        resources/textures/water-normal-texture-3.jpg
)

# Windows OpenGL support if needed
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 glu32)
endif()

# Silence warnings for volatiles
if (MSVC OR MSYS OR MINGW)
  add_compile_options(-Wno-volatile)
endif()

if (APPLE)
  add_compile_options(-Wno-deprecated-volatile)
endif()

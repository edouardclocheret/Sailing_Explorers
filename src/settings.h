#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 25;
    int shapeParameter2 = 25;
    float nearPlane = 0.01f;
    float farPlane = 100.0f;
    bool perPixelFilter = false;
    bool kernelBasedFilter = false;
    bool extraCredit1 = false;
    bool extraCredit2 = false;
    bool extraCredit3 = false;
    bool extraCredit4 = false;
    bool show_depth = false; //has replaced extra credit 1!!!
    bool show_velocity_x =false; //has replaced extra credit 2!!!
    bool show_velocity_y =false; //has replaced extra credit 3!!!
    bool enable_motion_blur = false; //has replaced extra credit 4!!!

    // Depth of Field settings (brought from Final Project Gear Up)
    bool enableDOF = false;
    float dofFocalDistance = 5.0f;
    float dofAperture = 2.0f;
    float dofMaxBlur = 10.0f;

    // L-System settings
    bool enableLSystem = true;
    bool enableInstancedForest = true;
    int lsystemType = 0; // 0=SimplePlant,1=Tree,2=Bush,3=Fern
    int lsystemIterations = 4;
    float lsystemScale = 1.0f;

    // Fog settings
    bool enableFog = true;
    float fogDensity = 0.50f;
    float fogStart = 0.0f;
    float fogEnd = 30.0f;
    int fogType = 0; // 0=Linear,1=Exp,2=Exp2
    float fogColorR = 0.7f;
    float fogColorG = 0.7f;
    float fogColorB = 0.8f;
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H

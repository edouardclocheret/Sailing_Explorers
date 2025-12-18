#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include "realtime.h"
#include "utils/aspectratiowidget/aspectratiowidget.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void connectUIElements();
    void connectParam1();
    void connectParam2();
    void connectNear();
    void connectFar();

    // From old Project 6
    // void connectPerPixelFilter();
    // void connectKernelBasedFilter();

    void connectUploadFile();
    void connectSaveImage();
    void connectExtraCredit();
    void connectDOF();
    void connectLSystem();
    void connectFog();

    Realtime *realtime;
    AspectRatioWidget *aspectRatioWidget;

    // From old Project 6
    // QCheckBox *filter1;
    // QCheckBox *filter2;

    QPushButton *uploadFile;
    QPushButton *saveImage;
    QPushButton *resetBoatButton;
    QLabel *scoreLabel;
    QLabel *arrowLabel;
    QPixmap arrowBasePixmap;
    QSlider *p1Slider;
    QSlider *p2Slider;
    QSpinBox *p1Box;
    QSpinBox *p2Box;
    QSlider *nearSlider;
    QSlider *farSlider;
    QDoubleSpinBox *nearBox;
    QDoubleSpinBox *farBox;

    // Collapsible controls
    QPushButton *collapseButton;
    QWidget *sideWidget;
    QScrollArea *sideScroll;

    // DOF controls
    QCheckBox *dofEnable;
    QDoubleSpinBox *dofFocalBox;
    QDoubleSpinBox *dofApertureBox;
    QDoubleSpinBox *dofMaxBlurBox;
    QSlider *dofFocalSlider;
    QSlider *dofApertureSlider;
    QSlider *dofMaxBlurSlider;

    // L-System controls
    QCheckBox *lsysEnable;
    QCheckBox *instancedForestEnable;
    QComboBox *lsysTypeCombo;
    QSpinBox *lsysIterBox;
    QDoubleSpinBox *lsysScaleBox;
    QSlider *lsysScaleSlider;

    // Fog controls
    QCheckBox *fogEnable;
    QComboBox *fogTypeCombo;
    QDoubleSpinBox *fogDensityBox;
    QDoubleSpinBox *fogStartBox;
    QDoubleSpinBox *fogEndBox;
    QDoubleSpinBox *fogRBox;
    QDoubleSpinBox *fogGBox;
    QDoubleSpinBox *fogBBox;
    QSlider *fogDensitySlider;
    QSlider *fogStartSlider;
    QSlider *fogEndSlider;
    QSlider *fogRSlider;
    QSlider *fogGSlider;
    QSlider *fogBSlider;

    // Extra Credit:
    QCheckBox *ec1;
    QCheckBox *ec2;
    QCheckBox *ec3;
    QCheckBox *ec4;

private slots:
    // From old Project 6
    // void onPerPixelFilter();
    // void onKernelBasedFilter();

    void onUploadFile();
    void onSaveImage();
    void onResetBoat();
    void onValChangeP1(int newValue);
    void onValChangeP2(int newValue);
    void onValChangeNearSlider(int newValue);
    void onValChangeFarSlider(int newValue);
    void onValChangeNearBox(double newValue);
    void onValChangeFarBox(double newValue);

    // Extra Credit:
    void onExtraCredit1();
    void onExtraCredit2();
    void onExtraCredit3();
    void onExtraCredit4();

    // DOF slots
    void onDOFToggled();
    void onDOFFocalChanged(double v);
    void onDOFApertureChanged(double v);
    void onDOFMaxBlurChanged(double v);

    // L-System slots
    void onLSystemToggled();
    void onInstancedForestToggled();
    void onLSystemTypeChanged(int idx);
    void onLSystemIterationsChanged(int v);
    void onLSystemScaleChanged(double v);

    // Fog slots
    void onFogToggled();
    void onFogTypeChanged(int idx);
    void onFogDensityChanged(double v);
    void onFogStartChanged(double v);
    void onFogEndChanged(double v);
    void onFogColorChanged();
    void onTreasureCollected(int score);
    void onTreasureDirectionChanged(float angleDegrees);
    void updateArrowPixmap(float angleDegrees);
};

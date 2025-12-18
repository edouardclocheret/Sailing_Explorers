#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QFile>
#include <QCoreApplication>
#include <QGroupBox>
#include <QPainter>
#include <iostream>

void MainWindow::initialize() {
    realtime = new Realtime;
    aspectRatioWidget = new AspectRatioWidget(this);
    aspectRatioWidget->setAspectWidget(realtime, 3.f/4.f);
    aspectRatioWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout *contentLayout = new QHBoxLayout; // holds controls + viewport
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    QVBoxLayout *vLayout = new QVBoxLayout(); // vertical alignment for controls
    vLayout->setAlignment(Qt::AlignTop);

    // Put the side controls inside a widget so we can collapse it
    sideWidget = new QWidget();
    sideWidget->setLayout(vLayout);

    collapseButton = new QPushButton("Control Panel");
    collapseButton->setCheckable(true);
    collapseButton->setChecked(false);

    // Make the controls area scrollable and hidden by default
    sideScroll = new QScrollArea();
    sideScroll->setWidget(sideWidget);
    sideScroll->setWidgetResizable(true);
    sideScroll->setFrameShape(QFrame::NoFrame);
    sideScroll->setVisible(false);
    sideScroll->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    contentLayout->addWidget(sideScroll);
    contentLayout->addWidget(aspectRatioWidget, 1);

    QFont hudFont;
    hudFont.setPointSize(20);
    hudFont.setBold(true);

    scoreLabel = new QLabel("Treasure Score: 0");
    scoreLabel->setFont(hudFont);
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setContentsMargins(0, 0, 0, 0);
    scoreLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    arrowLabel = new QLabel();
    arrowLabel->setFixedSize(80, 80);
    arrowLabel->setAlignment(Qt::AlignCenter);
    arrowLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    arrowBasePixmap = QPixmap(120, 120);
    arrowBasePixmap.fill(Qt::transparent);
    {
        QPainter painter(&arrowBasePixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        QColor fillColor(255, 215, 0);
        QColor outlineColor(120, 90, 0);
        painter.setBrush(fillColor);
        painter.setPen(QPen(outlineColor, 4));

        // Draw shaft
        QRectF shaftRect(50, 45, 20, 55);
        painter.drawRoundedRect(shaftRect, 6, 6);

        // Draw arrow head
        QPolygonF arrowPolygon;
        arrowPolygon << QPointF(60, 15) << QPointF(92, 62) << QPointF(60, 48) << QPointF(28, 62);
        painter.drawPolygon(arrowPolygon);
    }
    arrowLabel->setPixmap(arrowBasePixmap.scaled(arrowLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QHBoxLayout *topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 6, 0, 6);
    topBar->setSpacing(16);
    topBar->addStretch();
    topBar->addWidget(scoreLabel);
    topBar->addWidget(arrowLabel);
    topBar->addStretch();

    mainLayout->addLayout(topBar);

    // Create labels in sidebox
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    QLabel *tesselation_label = new QLabel(); // Parameters label
    tesselation_label->setText("Tesselation");
    tesselation_label->setFont(font);
    QLabel *camera_label = new QLabel(); // Camera label
    camera_label->setText("Camera");
    camera_label->setFont(font);

    // From old Project 6
    // QLabel *filters_label = new QLabel(); // Filters label
    // filters_label->setText("Filters");
    // filters_label->setFont(font);

    QLabel *ec_label = new QLabel(); // Extra Credit label
    ec_label->setText("Extra Credit");
    ec_label->setFont(font);
    QLabel *param1_label = new QLabel(); // Parameter 1 label
    param1_label->setText("Parameter 1:");
    QLabel *param2_label = new QLabel(); // Parameter 2 label
    param2_label->setText("Parameter 2:");
    QLabel *near_label = new QLabel(); // Near plane label
    near_label->setText("Near Plane:");
    QLabel *far_label = new QLabel(); // Far plane label
    far_label->setText("Far Plane:");


    // From old Project 6
    // // Create checkbox for per-pixel filter
    // filter1 = new QCheckBox();
    // filter1->setText(QStringLiteral("Per-Pixel Filter"));
    // filter1->setChecked(false);
    // // Create checkbox for kernel-based filter
    // filter2 = new QCheckBox();
    // filter2->setText(QStringLiteral("Kernel-Based Filter"));
    // filter2->setChecked(false);

    // Create file uploader for scene file
    uploadFile = new QPushButton(this);
    uploadFile->setText(QStringLiteral("Upload Scene File"));
    uploadFile->setVisible(false); // Hidden for demo build
    
    saveImage = new QPushButton(this);
    saveImage->setText(QStringLiteral("Save Image"));
    saveImage->setVisible(false); // Hidden for demo build

    resetBoatButton = new QPushButton(this);
    resetBoatButton->setText(QStringLiteral("Reset Position"));

    // Creates the boxes containing the parameter sliders and number boxes
    QGroupBox *p1Layout = new QGroupBox(); // horizonal slider 1 alignment
    QHBoxLayout *l1 = new QHBoxLayout();
    QGroupBox *p2Layout = new QGroupBox(); // horizonal slider 2 alignment
    QHBoxLayout *l2 = new QHBoxLayout();

    // Create slider controls to control parameters
    p1Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 1 slider
    p1Slider->setTickInterval(1);
    p1Slider->setMinimum(1);
    p1Slider->setMaximum(25);
    int initialP1 = settings.shapeParameter1;
    if (initialP1 < p1Slider->minimum()) initialP1 = p1Slider->minimum();
    if (initialP1 > p1Slider->maximum()) initialP1 = p1Slider->maximum();
    p1Slider->setValue(initialP1);

    p1Box = new QSpinBox();
    p1Box->setMinimum(1);
    p1Box->setMaximum(25);
    p1Box->setSingleStep(1);
    p1Box->setValue(initialP1);

    p2Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 2 slider
    p2Slider->setTickInterval(1);
    p2Slider->setMinimum(1);
    p2Slider->setMaximum(25);
    int initialP2 = settings.shapeParameter2;
    if (initialP2 < p2Slider->minimum()) initialP2 = p2Slider->minimum();
    if (initialP2 > p2Slider->maximum()) initialP2 = p2Slider->maximum();
    p2Slider->setValue(initialP2);

    p2Box = new QSpinBox();
    p2Box->setMinimum(1);
    p2Box->setMaximum(25);
    p2Box->setSingleStep(1);
    p2Box->setValue(initialP2);

    // Adds the slider and number box to the parameter layouts
    l1->addWidget(p1Slider);
    l1->addWidget(p1Box);
    p1Layout->setLayout(l1);

    l2->addWidget(p2Slider);
    l2->addWidget(p2Box);
    p2Layout->setLayout(l2);

    // Creates the boxes containing the camera sliders and number boxes
    QGroupBox *nearLayout = new QGroupBox(); // horizonal near slider alignment
    QHBoxLayout *lnear = new QHBoxLayout();
    QGroupBox *farLayout = new QGroupBox(); // horizonal far slider alignment
    QHBoxLayout *lfar = new QHBoxLayout();

    // Create slider controls to control near/far planes
    nearSlider = new QSlider(Qt::Orientation::Horizontal); // Near plane slider
    nearSlider->setTickInterval(1);
    nearSlider->setMinimum(1);
    nearSlider->setMaximum(1000);
    int initialNearSlider = static_cast<int>(settings.nearPlane * 100.f);
    if (initialNearSlider < nearSlider->minimum()) initialNearSlider = nearSlider->minimum();
    if (initialNearSlider > nearSlider->maximum()) initialNearSlider = nearSlider->maximum();
    nearSlider->setValue(initialNearSlider);

    nearBox = new QDoubleSpinBox();
    nearBox->setMinimum(0.01f);
    nearBox->setMaximum(10.f);
    nearBox->setSingleStep(0.1f);
    double initialNear = settings.nearPlane;
    if (initialNear < nearBox->minimum()) initialNear = nearBox->minimum();
    if (initialNear > nearBox->maximum()) initialNear = nearBox->maximum();
    nearBox->setValue(initialNear);

    farSlider = new QSlider(Qt::Orientation::Horizontal); // Far plane slider
    farSlider->setTickInterval(1);
    farSlider->setMinimum(1000);
    farSlider->setMaximum(20000);
    int initialFarSlider = static_cast<int>(settings.farPlane * 100.f);
    if (initialFarSlider < farSlider->minimum()) initialFarSlider = farSlider->minimum();
    if (initialFarSlider > farSlider->maximum()) initialFarSlider = farSlider->maximum();
    farSlider->setValue(initialFarSlider);

    farBox = new QDoubleSpinBox();
    farBox->setMinimum(10.f);
    farBox->setMaximum(200.f);
    farBox->setSingleStep(0.1f);
    double initialFar = settings.farPlane;
    if (initialFar < farBox->minimum()) initialFar = farBox->minimum();
    if (initialFar > farBox->maximum()) initialFar = farBox->maximum();
    farBox->setValue(initialFar);

    // Adds the slider and number box to the parameter layouts
    lnear->addWidget(nearSlider);
    lnear->addWidget(nearBox);
    nearLayout->setLayout(lnear);

    lfar->addWidget(farSlider);
    lfar->addWidget(farBox);
    farLayout->setLayout(lfar);

    // Extra Credit:
    ec1 = new QCheckBox();
    ec1->setText(QStringLiteral("Show depth"));
    std::cout << "Checkbox text is now: " << ec1->text().toStdString() << std::endl;

    ec1->setChecked(false);

    ec2 = new QCheckBox();
    ec2->setText(QStringLiteral("Show x-speed"));
    ec2->setChecked(false);

    ec3 = new QCheckBox();
    ec3->setText(QStringLiteral("Show y-speed"));
    ec3->setChecked(false);

    ec4 = new QCheckBox();
    ec4->setText(QStringLiteral("Enable motion blur"));
    ec4->setChecked(false);

    // --- DOF controls ---
    dofEnable = new QCheckBox();
    dofEnable->setText(QStringLiteral("Enable DOF"));
    dofEnable->setChecked(false);

    dofFocalBox = new QDoubleSpinBox();
    dofFocalBox->setMinimum(0.1);
    dofFocalBox->setMaximum(100.0);
    dofFocalBox->setValue(5.0);

    // Slider for DOF focal distance: map slider int [1..1000] -> double value/10
    dofFocalSlider = new QSlider(Qt::Orientation::Horizontal);
    dofFocalSlider->setMinimum(1);
    dofFocalSlider->setMaximum(1000);
    dofFocalSlider->setValue(static_cast<int>(dofFocalBox->value()*10.0));

    dofApertureBox = new QDoubleSpinBox();
    dofApertureBox->setMinimum(0.01);
    dofApertureBox->setMaximum(10.0);
    dofApertureBox->setValue(2.0);

    // Slider for DOF aperture: map slider int [1..1000] -> double value/100
    dofApertureSlider = new QSlider(Qt::Orientation::Horizontal);
    dofApertureSlider->setMinimum(1);
    dofApertureSlider->setMaximum(1000);
    dofApertureSlider->setValue(static_cast<int>(dofApertureBox->value()*100.0));

    dofMaxBlurBox = new QDoubleSpinBox();
    dofMaxBlurBox->setMinimum(0.0);
    dofMaxBlurBox->setMaximum(100.0);
    dofMaxBlurBox->setValue(10.0);

    // Slider for DOF max blur: map slider int [0..1000] -> double value/10
    dofMaxBlurSlider = new QSlider(Qt::Orientation::Horizontal);
    dofMaxBlurSlider->setMinimum(0);
    dofMaxBlurSlider->setMaximum(1000);
    dofMaxBlurSlider->setValue(static_cast<int>(dofMaxBlurBox->value()*10.0));

    // --- L-System controls ---
    lsysEnable = new QCheckBox();
    lsysEnable->setText(QStringLiteral("Enable L-System"));
    lsysEnable->setChecked(settings.enableLSystem);

    instancedForestEnable = new QCheckBox();
    instancedForestEnable->setText(QStringLiteral("Enable Instanced Forest"));
    instancedForestEnable->setChecked(settings.enableInstancedForest);

    lsysTypeCombo = new QComboBox();
    lsysTypeCombo->addItem("SimplePlant");
    lsysTypeCombo->addItem("Tree");
    lsysTypeCombo->addItem("Bush");
    lsysTypeCombo->addItem("Fern");

    lsysIterBox = new QSpinBox();
    lsysIterBox->setMinimum(1);
    lsysIterBox->setMaximum(8);
    lsysIterBox->setValue(4);

    lsysScaleBox = new QDoubleSpinBox();
    lsysScaleBox->setMinimum(0.01);
    lsysScaleBox->setMaximum(10.0);
    lsysScaleBox->setValue(1.0);

    // Slider for L-System scale: map slider int [1..1000] -> double value/100
    lsysScaleSlider = new QSlider(Qt::Orientation::Horizontal);
    lsysScaleSlider->setMinimum(1);
    lsysScaleSlider->setMaximum(1000);
    lsysScaleSlider->setValue(static_cast<int>(lsysScaleBox->value()*100.0));

    // --- Fog controls ---
    fogEnable = new QCheckBox();
    fogEnable->setText(QStringLiteral("Enable Fog"));
    fogEnable->setChecked(settings.enableFog);

    fogTypeCombo = new QComboBox();
    fogTypeCombo->addItem("Linear");
    fogTypeCombo->addItem("Exponential");
    fogTypeCombo->addItem("Exponential^2");

    fogDensityBox = new QDoubleSpinBox();
    fogDensityBox->setMinimum(0.0);
    fogDensityBox->setMaximum(1.0);
    fogDensityBox->setSingleStep(0.01);
    double initialFogDensity = settings.fogDensity;
    if (initialFogDensity < fogDensityBox->minimum()) initialFogDensity = fogDensityBox->minimum();
    if (initialFogDensity > fogDensityBox->maximum()) initialFogDensity = fogDensityBox->maximum();
    fogDensityBox->setValue(initialFogDensity);

    // Slider for fog density: [0..100] maps to [0.00..1.00]
    fogDensitySlider = new QSlider(Qt::Orientation::Horizontal);
    fogDensitySlider->setMinimum(0);
    fogDensitySlider->setMaximum(100);
    fogDensitySlider->setValue(static_cast<int>(initialFogDensity*100.0));

    fogStartBox = new QDoubleSpinBox();
    fogStartBox->setMinimum(0.0);
    fogStartBox->setMaximum(100.0);
    double initialFogStart = settings.fogStart;
    if (initialFogStart < fogStartBox->minimum()) initialFogStart = fogStartBox->minimum();
    if (initialFogStart > fogStartBox->maximum()) initialFogStart = fogStartBox->maximum();
    fogStartBox->setValue(initialFogStart);

    // Slider for fog start: [0..10000] maps directly to double value
    fogStartSlider = new QSlider(Qt::Orientation::Horizontal);
    fogStartSlider->setMinimum(0);
    fogStartSlider->setMaximum(10000);
    fogStartSlider->setValue(static_cast<int>(initialFogStart));

    fogEndBox = new QDoubleSpinBox();
    fogEndBox->setMinimum(0.0);
    fogEndBox->setMaximum(10000.0);
    double initialFogEnd = settings.fogEnd;
    if (initialFogEnd < fogEndBox->minimum()) initialFogEnd = fogEndBox->minimum();
    if (initialFogEnd > fogEndBox->maximum()) initialFogEnd = fogEndBox->maximum();
    fogEndBox->setValue(initialFogEnd);

    // Slider for fog end: [0..10000]
    fogEndSlider = new QSlider(Qt::Orientation::Horizontal);
    fogEndSlider->setMinimum(0);
    fogEndSlider->setMaximum(100);
    fogEndSlider->setValue(static_cast<int>(initialFogEnd));

    fogRBox = new QDoubleSpinBox();
    fogRBox->setMinimum(0.0); fogRBox->setMaximum(1.0); fogRBox->setSingleStep(0.01); fogRBox->setValue(0.7);
    fogGBox = new QDoubleSpinBox();
    fogGBox->setMinimum(0.0); fogGBox->setMaximum(1.0); fogGBox->setSingleStep(0.01); fogGBox->setValue(0.7);
    fogBBox = new QDoubleSpinBox();
    fogBBox->setMinimum(0.0); fogBBox->setMaximum(1.0); fogBBox->setSingleStep(0.01); fogBBox->setValue(0.8);

    // Sliders for fog color components: [0..100] -> [0.00..1.00]
    fogRSlider = new QSlider(Qt::Orientation::Horizontal);
    fogRSlider->setMinimum(0); fogRSlider->setMaximum(100); fogRSlider->setValue(static_cast<int>(fogRBox->value()*100.0));
    fogGSlider = new QSlider(Qt::Orientation::Horizontal);
    fogGSlider->setMinimum(0); fogGSlider->setMaximum(100); fogGSlider->setValue(static_cast<int>(fogGBox->value()*100.0));
    fogBSlider = new QSlider(Qt::Orientation::Horizontal);
    fogBSlider->setMinimum(0); fogBSlider->setMaximum(100); fogBSlider->setValue(static_cast<int>(fogBBox->value()*100.0));

    vLayout->addWidget(tesselation_label);
    vLayout->addWidget(param1_label);
    vLayout->addWidget(p1Layout);
    vLayout->addWidget(param2_label);
    vLayout->addWidget(p2Layout);
    vLayout->addWidget(camera_label);
    vLayout->addWidget(near_label);
    vLayout->addWidget(nearLayout);
    vLayout->addWidget(far_label);
    vLayout->addWidget(farLayout);

    // From old Project 6
    // vLayout->addWidget(filters_label);
    // vLayout->addWidget(filter1);
    // vLayout->addWidget(filter2);

    // Extra Credit:
    vLayout->addWidget(ec_label);
    vLayout->addWidget(ec1);
    vLayout->addWidget(ec2);
    vLayout->addWidget(ec3);
    vLayout->addWidget(ec4);

    // Add DOF controls (slider + numeric box per row)
    QLabel *dof_label = new QLabel("Depth of Field");
    dof_label->setFont(font);
    vLayout->addWidget(dof_label);
    vLayout->addWidget(dofEnable);
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Focal Distance:"));
        h->addWidget(dofFocalSlider, 1);
        h->addWidget(dofFocalBox);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Aperture:"));
        h->addWidget(dofApertureSlider, 1);
        h->addWidget(dofApertureBox);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Max Blur (px):"));
        h->addWidget(dofMaxBlurSlider, 1);
        h->addWidget(dofMaxBlurBox);
        vLayout->addLayout(h);
    }

    // Add L-System controls
    QLabel *lsys_label = new QLabel("L-Systems");
    lsys_label->setFont(font);
    vLayout->addWidget(lsys_label);
    vLayout->addWidget(lsysEnable);
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Type:"));
        h->addWidget(lsysTypeCombo);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Iterations:"));
        h->addWidget(lsysIterBox);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Scale:"));
        h->addWidget(lsysScaleSlider, 1);
        h->addWidget(lsysScaleBox);
        vLayout->addLayout(h);
    }

    QLabel *instanced_label = new QLabel("Instanced Forest");
    instanced_label->setFont(font);
    vLayout->addWidget(instanced_label);
    vLayout->addWidget(instancedForestEnable);

    // Add Fog controls
    QLabel *fog_label = new QLabel("Fog");
    fog_label->setFont(font);
    vLayout->addWidget(fog_label);
    vLayout->addWidget(fogEnable);
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Type:"));
        h->addWidget(fogTypeCombo);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Density:"));
        h->addWidget(fogDensitySlider, 1);
        h->addWidget(fogDensityBox);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("Start:"));
        h->addWidget(fogStartSlider, 1);
        h->addWidget(fogStartBox);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(new QLabel("End:"));
        h->addWidget(fogEndSlider, 1);
        h->addWidget(fogEndBox);
        vLayout->addLayout(h);
    }
    vLayout->addWidget(new QLabel("Fog Color R/G/B:"));
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(fogRSlider, 1); h->addWidget(fogRBox);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(fogGSlider, 1); h->addWidget(fogGBox);
        vLayout->addLayout(h);
    }
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->addWidget(fogBSlider, 1); h->addWidget(fogBBox);
        vLayout->addLayout(h);
    }

    mainLayout->addLayout(contentLayout, 1);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setAlignment(Qt::AlignCenter);
    bottomLayout->setSpacing(12);
    bottomLayout->addStretch();
    bottomLayout->addWidget(collapseButton);
    bottomLayout->addWidget(resetBoatButton);
    bottomLayout->addStretch();

    mainLayout->addLayout(bottomLayout);

    this->setLayout(mainLayout);

    connectUIElements();

    // Connect collapse button: toggle the visibility of the scroll area containing controls
    connect(collapseButton, &QPushButton::toggled, sideScroll, &QWidget::setVisible);

    // Connect newly added controls
    connectDOF();
    connectLSystem();
    connectFog();

    // Initialize controls from the current settings defaults
    onValChangeP1(settings.shapeParameter1);
    onValChangeP2(settings.shapeParameter2);

    onValChangeNearBox(settings.nearPlane);
    onValChangeFarBox(settings.farPlane);

    // Auto-load sailboat scene on startup if none selected
    if (settings.sceneFilePath.empty()) {
        // Try multiple candidate locations for sailboat.json and pick the first that exists.
        QString candidates[] = {
            // working dir relative
            QDir::currentPath() + QDir::separator() + QString("scenefiles/realtime/sail_game/sailboat.json"),
            // executable dir (build/debug) -> try parent dirs
            QCoreApplication::applicationDirPath() + QDir::separator() + QString("../scenefiles/realtime/sail_game/sailboat.json"),
            QCoreApplication::applicationDirPath() + QDir::separator() + QString("../../scenefiles/realtime/sail_game/sailboat.json"),
            QCoreApplication::applicationDirPath() + QDir::separator() + QString("../../../scenefiles/realtime/sail_game/sailboat.json")
        };
        for (const QString &cand : candidates) {
            if (QFile::exists(cand)) {
                settings.sceneFilePath = cand.toStdString();
                realtime->sceneChanged();
                break;
            }
        }
    }
}

void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

void MainWindow::connectUIElements() {
    // From old Project 6
    //connectPerPixelFilter();
    //connectKernelBasedFilter();
    // connectUploadFile(); // Disabled to prevent loading alternate scenes
    connectSaveImage();
    connect(resetBoatButton, &QPushButton::clicked, this, &MainWindow::onResetBoat);
    connectParam1();
    connectParam2();
    connectNear();
    connectFar();
    connectExtraCredit();
    connect(realtime, &Realtime::treasureCollected, this, &MainWindow::onTreasureCollected);
    connect(realtime, &Realtime::treasureDirectionChanged, this, &MainWindow::onTreasureDirectionChanged);
}


// From old Project 6
// void MainWindow::connectPerPixelFilter() {
//     connect(filter1, &QCheckBox::clicked, this, &MainWindow::onPerPixelFilter);
// }
// void MainWindow::connectKernelBasedFilter() {
//     connect(filter2, &QCheckBox::clicked, this, &MainWindow::onKernelBasedFilter);
// }

void MainWindow::connectUploadFile() {
    connect(uploadFile, &QPushButton::clicked, this, &MainWindow::onUploadFile);
}

void MainWindow::connectSaveImage() {
    connect(saveImage, &QPushButton::clicked, this, &MainWindow::onSaveImage);
}

void MainWindow::connectParam1() {
    connect(p1Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP1);
    connect(p1Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP1);
}

void MainWindow::connectParam2() {
    connect(p2Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP2);
    connect(p2Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP2);
}

void MainWindow::connectNear() {
    connect(nearSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeNearSlider);
    connect(nearBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeNearBox);
}

void MainWindow::connectFar() {
    connect(farSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeFarSlider);
    connect(farBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeFarBox);
}

void MainWindow::connectExtraCredit() {
    connect(ec1, &QCheckBox::clicked, this, &MainWindow::onExtraCredit1);
    connect(ec2, &QCheckBox::clicked, this, &MainWindow::onExtraCredit2);
    connect(ec3, &QCheckBox::clicked, this, &MainWindow::onExtraCredit3);
    connect(ec4, &QCheckBox::clicked, this, &MainWindow::onExtraCredit4);
}

void MainWindow::connectDOF() {
    connect(dofEnable, &QCheckBox::clicked, this, &MainWindow::onDOFToggled);
    connect(dofFocalBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onDOFFocalChanged);
    connect(dofApertureBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onDOFApertureChanged);
    connect(dofMaxBlurBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onDOFMaxBlurChanged);

    // Wire sliders <-> spinboxes for DOF
    connect(dofFocalSlider, &QSlider::valueChanged, this, [this](int v){ dofFocalBox->setValue(v/10.0); });
    connect(dofFocalBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ dofFocalSlider->setValue(static_cast<int>(v*10.0)); });

    connect(dofApertureSlider, &QSlider::valueChanged, this, [this](int v){ dofApertureBox->setValue(v/100.0); });
    connect(dofApertureBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ dofApertureSlider->setValue(static_cast<int>(v*100.0)); });

    connect(dofMaxBlurSlider, &QSlider::valueChanged, this, [this](int v){ dofMaxBlurBox->setValue(v/10.0); });
    connect(dofMaxBlurBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ dofMaxBlurSlider->setValue(static_cast<int>(v*10.0)); });
}

void MainWindow::connectLSystem() {
    connect(lsysEnable, &QCheckBox::clicked, this, &MainWindow::onLSystemToggled);
    connect(instancedForestEnable, &QCheckBox::clicked, this, &MainWindow::onInstancedForestToggled);
    connect(lsysTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, &MainWindow::onLSystemTypeChanged);
    connect(lsysIterBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
        this, &MainWindow::onLSystemIterationsChanged);
    connect(lsysScaleBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onLSystemScaleChanged);

    // L-System scale slider <-> spinbox
    connect(lsysScaleSlider, &QSlider::valueChanged, this, [this](int v){ lsysScaleBox->setValue(v/100.0); });
    connect(lsysScaleBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ lsysScaleSlider->setValue(static_cast<int>(v*100.0)); });
}

void MainWindow::connectFog() {
    connect(fogEnable, &QCheckBox::clicked, this, &MainWindow::onFogToggled);
    connect(fogTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, &MainWindow::onFogTypeChanged);
    connect(fogDensityBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onFogDensityChanged);
    connect(fogStartBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onFogStartChanged);
    connect(fogEndBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onFogEndChanged);
    // color changes handled by three boxes - connect them to a single handler
    connect(fogRBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onFogColorChanged);
    connect(fogGBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onFogColorChanged);
    connect(fogBBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::onFogColorChanged);

    // Wire fog sliders <-> spinboxes
    connect(fogDensitySlider, &QSlider::valueChanged, this, [this](int v){ fogDensityBox->setValue(v/100.0); });
    connect(fogDensityBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ fogDensitySlider->setValue(static_cast<int>(v*100.0)); });

    connect(fogStartSlider, &QSlider::valueChanged, this, [this](int v){ fogStartBox->setValue(static_cast<double>(v)); });
    connect(fogStartBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ fogStartSlider->setValue(static_cast<int>(v)); });

    connect(fogEndSlider, &QSlider::valueChanged, this, [this](int v){ fogEndBox->setValue(static_cast<double>(v)); });
    connect(fogEndBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ fogEndSlider->setValue(static_cast<int>(v)); });

    connect(fogRSlider, &QSlider::valueChanged, this, [this](int v){ fogRBox->setValue(v/100.0); });
    connect(fogGSlider, &QSlider::valueChanged, this, [this](int v){ fogGBox->setValue(v/100.0); });
    connect(fogBSlider, &QSlider::valueChanged, this, [this](int v){ fogBBox->setValue(v/100.0); });

    connect(fogRBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ fogRSlider->setValue(static_cast<int>(v*100.0)); });
    connect(fogGBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ fogGSlider->setValue(static_cast<int>(v*100.0)); });
    connect(fogBBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        this, [this](double v){ fogBSlider->setValue(static_cast<int>(v*100.0)); });
}

// From old Project 6
// void MainWindow::onPerPixelFilter() {
//     settings.perPixelFilter = !settings.perPixelFilter;
//     realtime->settingsChanged();
// }
// void MainWindow::onKernelBasedFilter() {
//     settings.kernelBasedFilter = !settings.kernelBasedFilter;
//     realtime->settingsChanged();
// }

void MainWindow::onUploadFile() {
    // Scene loading is intentionally disabled for demos. The original
    // implementation is preserved below for future reference if this feature
    // needs to be re-enabled.
    /*
    // Get abs path of scene file
    QString configFilePath = QFileDialog::getOpenFileName(this, tr("Upload File"),
                                                          QDir::currentPath()
                                                              .append(QDir::separator())
                                                              .append("scenefiles")
                                                              .append(QDir::separator())
                                                              .append("realtime")
                                                              .append(QDir::separator())
                                                              .append("required"), tr("Scene Files (*.json)"));
    if (configFilePath.isNull()) {
        std::cout << "Failed to load null scenefile." << std::endl;
        return;
    }

    settings.sceneFilePath = configFilePath.toStdString();

    std::cout << "Loaded scenefile: \"" << configFilePath.toStdString() << "\"." << std::endl;

    realtime->sceneChanged();
    */
}

void MainWindow::onSaveImage() {
    if (settings.sceneFilePath.empty()) {
        std::cout << "No scene file loaded." << std::endl;
        return;
    }
    std::string sceneName = settings.sceneFilePath.substr(0, settings.sceneFilePath.find_last_of("."));
    sceneName = sceneName.substr(sceneName.find_last_of("/")+1);
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Image"),
                                                    QDir::currentPath()
                                                        .append(QDir::separator())
                                                        .append("student_outputs")
                                                        .append(QDir::separator())
                                                        .append("realtime")
                                                        .append(QDir::separator())
                                                        .append("required")
                                                        .append(QDir::separator())
                                                        .append(sceneName), tr("Image Files (*.png)"));
    std::cout << "Saving image to: \"" << filePath.toStdString() << "\"." << std::endl;
    realtime->saveViewportImage(filePath.toStdString());
}

void MainWindow::onResetBoat() {
    if (realtime) {
        realtime->resetBoatAndCamera();
    }
}

void MainWindow::onValChangeP1(int newValue) {
    p1Slider->setValue(newValue);
    p1Box->setValue(newValue);
    settings.shapeParameter1 = p1Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeP2(int newValue) {
    p2Slider->setValue(newValue);
    p2Box->setValue(newValue);
    settings.shapeParameter2 = p2Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearSlider(int newValue) {
    //nearSlider->setValue(newValue);
    nearBox->setValue(newValue/100.f);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarSlider(int newValue) {
    //farSlider->setValue(newValue);
    farBox->setValue(newValue/100.f);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearBox(double newValue) {
    nearSlider->setValue(int(newValue*100.f));
    //nearBox->setValue(newValue);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarBox(double newValue) {
    farSlider->setValue(int(newValue*100.f));
    //farBox->setValue(newValue);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

// Extra Credit:

void MainWindow::onExtraCredit1() {
    settings.show_depth = !settings.show_depth;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit2() {
    settings.show_velocity_x = !settings.show_velocity_x;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit3() {
    settings.show_velocity_y = !settings.show_velocity_y;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit4() {
    settings.enable_motion_blur = !settings.enable_motion_blur;
    realtime->settingsChanged();
}

// DOF handlers
void MainWindow::onDOFToggled() {
    settings.enableDOF = !settings.enableDOF;
    realtime->settingsChanged();
}

void MainWindow::onDOFFocalChanged(double v) {
    settings.dofFocalDistance = static_cast<float>(v);
    realtime->settingsChanged();
}

void MainWindow::onDOFApertureChanged(double v) {
    settings.dofAperture = static_cast<float>(v);
    realtime->settingsChanged();
}

void MainWindow::onDOFMaxBlurChanged(double v) {
    settings.dofMaxBlur = static_cast<float>(v);
    realtime->settingsChanged();
}

// L-System handlers
void MainWindow::onLSystemToggled() {
    settings.enableLSystem = lsysEnable->isChecked();
    realtime->settingsChanged();
}

void MainWindow::onInstancedForestToggled() {
    settings.enableInstancedForest = instancedForestEnable->isChecked();
    realtime->settingsChanged();
}

void MainWindow::onLSystemTypeChanged(int idx) {
    settings.lsystemType = idx;
    realtime->settingsChanged();
}

void MainWindow::onLSystemIterationsChanged(int v) {
    settings.lsystemIterations = v;
    realtime->settingsChanged();
}

void MainWindow::onLSystemScaleChanged(double v) {
    settings.lsystemScale = static_cast<float>(v);
    realtime->settingsChanged();
}

// Fog handlers
void MainWindow::onFogToggled() {
    settings.enableFog = fogEnable->isChecked();
    realtime->settingsChanged();
}

void MainWindow::onFogTypeChanged(int idx) {
    settings.fogType = idx;
    realtime->settingsChanged();
}

void MainWindow::onFogDensityChanged(double v) {
    settings.fogDensity = static_cast<float>(v);
    realtime->settingsChanged();
}

void MainWindow::onFogStartChanged(double v) {
    settings.fogStart = static_cast<float>(v);
    realtime->settingsChanged();
}

void MainWindow::onFogEndChanged(double v) {
    settings.fogEnd = static_cast<float>(v);
    realtime->settingsChanged();
}

void MainWindow::onFogColorChanged() {
    settings.fogColorR = static_cast<float>(fogRBox->value());
    settings.fogColorG = static_cast<float>(fogGBox->value());
    settings.fogColorB = static_cast<float>(fogBBox->value());
    realtime->settingsChanged();
}

void MainWindow::onTreasureCollected(int score) {
    if (!scoreLabel) return;
    scoreLabel->setText(QStringLiteral("Treasure Score: %1").arg(score));
}

void MainWindow::onTreasureDirectionChanged(float angleDegrees) {
    updateArrowPixmap(angleDegrees);
}

void MainWindow::updateArrowPixmap(float angleDegrees) {
    if (!arrowLabel || arrowBasePixmap.isNull()) {
        return;
    }
    QTransform transform;
    transform.rotate(-angleDegrees);
    QPixmap rotated = arrowBasePixmap.transformed(transform, Qt::SmoothTransformation);
    QPixmap scaled = rotated.scaled(arrowLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    arrowLabel->setPixmap(scaled);
}

#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QKeyEvent>
#include <QDateTime>
#include <QPixmap>
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <iostream>
#include "pff.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), remainingSeconds(0) {
    
    setWindowTitle("Antigravity Camera");
    resize(960, 640);
    
    // Set global stylesheet for the window
    setStyleSheet("QMainWindow { background-color: #121214; }");

    // Initialize timers
    countdownTimer = new QTimer(this);
    countdownTimer->setInterval(1000);
    connect(countdownTimer, &QTimer::timeout, this, &MainWindow::onCountdownTick);

    setupUi();
    
    // Initialize settings menu
    settingsMenu = new SettingsMenu(this);
    connect(settingsMenu, &SettingsMenu::filterChanged, cameraView, &CameraView::setFilterType);

    // Scan for and load the last taken photo thumbnail
    loadLastTakenPhotoThumbnail();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // ==========================================
    // SCREEN 1: CAMERA UI
    // ==========================================
    cameraScreen = new QWidget(this);
    QHBoxLayout *camLayout = new QHBoxLayout(cameraScreen);
    camLayout->setContentsMargins(0, 0, 0, 0);
    camLayout->setSpacing(0);

    // Left Sidebar panel
    QWidget *sidebar = new QWidget(cameraScreen);
    sidebar->setFixedWidth(90);
    sidebar->setStyleSheet(
        "QWidget {"
        "  background-color: #1a1a1e;"
        "  border-right: 1px solid #252528;"
        "}"
        "QPushButton {"
        "  border: none;"
        "  outline: none;"
        "}"
    );
    
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(0, 30, 0, 30);
    sideLayout->setSpacing(24);

    // Button 1: Settings (Vertical 3 dots)
    btnSettings = new QPushButton("⋮", sidebar);
    btnSettings->setFixedSize(50, 50);
    btnSettings->setStyleSheet(
        "QPushButton {"
        "  color: #a0a0a5;"
        "  font-size: 28px;"
        "  background-color: transparent;"
        "  border-radius: 25px;"
        "}"
        "QPushButton:hover {"
        "  color: #ffffff;"
        "  background-color: #28282c;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #38383c;"
        "}"
    );
    connect(btnSettings, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    sideLayout->addWidget(btnSettings, 0, Qt::AlignHCenter);

    sideLayout->addStretch();

    // Button 2: Shutter Button (White circle)
    btnShutter = new QPushButton(sidebar);
    btnShutter->setFixedSize(60, 60);
    btnShutter->setStyleSheet(
        "QPushButton {"
        "  background-color: #ffffff;"
        "  border: 4px solid #1a1a1e;"
        "  border-radius: 30px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #e5e5e5;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #cccccc;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #555558;"
        "}"
    );
    connect(btnShutter, &QPushButton::clicked, this, &MainWindow::onShutterClicked);
    sideLayout->addWidget(btnShutter, 0, Qt::AlignHCenter);

    sideLayout->addStretch();

    // Button 3: Gallery/Thumbnail (Rounded square)
    btnThumbnail = new QPushButton(sidebar);
    btnThumbnail->setFixedSize(54, 54);
    btnThumbnail->setStyleSheet(
        "QPushButton {"
        "  background-color: #0c0c0e;"
        "  border: 2px solid #2d2d32;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  border-color: #007acc;"
        "}"
    );
    connect(btnThumbnail, &QPushButton::clicked, this, &MainWindow::onGalleryClicked);
    sideLayout->addWidget(btnThumbnail, 0, Qt::AlignHCenter);

    camLayout->addWidget(sidebar);

    // Center/Right Camera View container
    QWidget *previewContainer = new QWidget(cameraScreen);
    QVBoxLayout *previewLayout = new QVBoxLayout(previewContainer);
    previewLayout->setContentsMargins(24, 24, 24, 24);

    // Create a beautiful wrapper card with shadow/borders
    QFrame *previewCard = new QFrame(previewContainer);
    previewCard->setStyleSheet(
        "QFrame {"
        "  background-color: #0c0c0e;"
        "  border: 1px solid #252528;"
        "  border-radius: 16px;"
        "}"
    );
    
    QVBoxLayout *cardLayout = new QVBoxLayout(previewCard);
    cardLayout->setContentsMargins(8, 8, 8, 8);

    cameraView = new CameraView(previewCard);
    cardLayout->addWidget(cameraView);

    // Apply soft drop shadow to the viewport card
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(previewCard);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 150));
    shadow->setOffset(0, 4);
    previewCard->setGraphicsEffect(shadow);

    previewLayout->addWidget(previewCard);
    camLayout->addWidget(previewContainer);

    stackedWidget->addWidget(cameraScreen);

    // ==========================================
    // SCREEN 2: IMAGE VIEWER
    // ==========================================
    viewerScreen = new QWidget(this);
    viewerScreen->setStyleSheet("QWidget { background-color: #08080a; }");
    
    QVBoxLayout *viewLayout = new QVBoxLayout(viewerScreen);
    viewLayout->setContentsMargins(20, 20, 20, 20);
    viewLayout->setSpacing(16);

    // Top control bar
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    
    lblViewerTimestamp = new QLabel(viewerScreen);
    lblViewerTimestamp->setStyleSheet(
        "color: #a0a0a5;"
        "font-size: 14px;"
        "font-family: 'monospace';"
        "font-weight: bold;"
    );
    topBarLayout->addWidget(lblViewerTimestamp);
    topBarLayout->addStretch();

    btnCloseViewer = new QPushButton("✕", viewerScreen);
    btnCloseViewer->setFixedSize(36, 36);
    btnCloseViewer->setStyleSheet(
        "QPushButton {"
        "  color: #ffffff;"
        "  background-color: #1e1e22;"
        "  border: 1px solid #2d2d32;"
        "  border-radius: 18px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  outline: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: #c9302c;"
        "  border-color: #ac2925;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #761c19;"
        "}"
    );
    connect(btnCloseViewer, &QPushButton::clicked, this, &MainWindow::onClosePreviewClicked);
    topBarLayout->addWidget(btnCloseViewer);
    
    viewLayout->addLayout(topBarLayout);

    // Image viewer display area
    QFrame *imageFrame = new QFrame(viewerScreen);
    imageFrame->setStyleSheet(
        "QFrame {"
        "  background-color: #040405;"
        "  border: 1px solid #1c1c1f;"
        "  border-radius: 12px;"
        "}"
    );
    QVBoxLayout *imageFrameLayout = new QVBoxLayout(imageFrame);
    imageFrameLayout->setContentsMargins(0, 0, 0, 0);

    lblViewerImage = new QLabel(imageFrame);
    lblViewerImage->setAlignment(Qt::AlignCenter);
    imageFrameLayout->addWidget(lblViewerImage);

    viewLayout->addWidget(imageFrame, 1);

    stackedWidget->addWidget(viewerScreen);
}

void MainWindow::onSettingsClicked() {
    // Show settings menu positioned relative to settings button
    QPoint btnPos = btnSettings->mapToGlobal(QPoint(0, 0));
    settingsMenu->move(btnPos.x() + btnSettings->width() + 10, btnPos.y());
    settingsMenu->show();
}

void MainWindow::onShutterClicked() {
    int timerSeconds = settingsMenu->getTimerSeconds();

    if (timerSeconds == 0) {
        capturePhoto();
    } else {
        // Start countdown process
        remainingSeconds = timerSeconds;
        cameraView->setCountdownValue(remainingSeconds);
        
        btnShutter->setEnabled(false);
        btnSettings->setEnabled(false);
        
        countdownTimer->start();
    }
}

void MainWindow::onCountdownTick() {
    remainingSeconds--;
    if (remainingSeconds > 0) {
        cameraView->setCountdownValue(remainingSeconds);
    } else {
        countdownTimer->stop();
        cameraView->setCountdownValue(-1);
        
        btnShutter->setEnabled(true);
        btnSettings->setEnabled(true);
        
        // Take photo
        capturePhoto();
    }
}

void MainWindow::capturePhoto() {
    // 1. Grab current image from viewport
    QImage frame = cameraView->captureCurrentFrame();
    if (frame.isNull()) {
        QMessageBox::warning(this, "Capture Error", "Failed to capture frame from preview feed.");
        return;
    }

    // 2. Prepare My Images folder
    QString outDir = "/home/zenx/Desktop/My Images";
    QDir().mkpath(outDir);

    // 3. Construct filename
    QString timestampStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("IMG_%1.pff").arg(timestampStr);
    QString fullPath = QDir(outDir).filePath(filename);

    // 4. Encode to PFF format
    pff::Image pffImg;
    std::memcpy(pffImg.header.magic, "PFF!", 4);
    pffImg.header.width = frame.width();
    pffImg.header.height = frame.height();
    pffImg.header.channels = 3; // RGB
    pffImg.header.filter_type = static_cast<uint8_t>(settingsMenu->getFilterType());
    pffImg.header.compression = settingsMenu->getRleEnabled() ? pff::COMPRESS_RLE : pff::COMPRESS_NONE;
    pffImg.header.timestamp = QDateTime::currentSecsSinceEpoch();
    std::memset(pffImg.header.reserved, 0, sizeof(pffImg.header.reserved));

    // Copy frame buffer
    int byteCount = frame.width() * frame.height() * 3;
    pffImg.pixels.resize(byteCount);
    std::memcpy(pffImg.pixels.data(), frame.bits(), byteCount);

    // Save file
    if (!pff::save_pff(fullPath.toStdString(), pffImg)) {
        QMessageBox::warning(this, "Save Error", "Failed to write picture to disk in PFF format.");
        return;
    }

    // 5. Trigger shutter flash effect
    cameraView->triggerFlash();

    // 6. Cache as last photo and update thumbnail button
    lastPhotoPath = fullPath;
    lastPhotoImg = frame;

    // Create scaled thumbnail pixmap
    QPixmap thumb = QPixmap::fromImage(frame).scaled(54, 54, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    btnThumbnail->setIcon(QIcon(thumb));
    btnThumbnail->setIconSize(QSize(50, 50));
}

void MainWindow::onGalleryClicked() {
    if (lastPhotoPath.isEmpty() || lastPhotoImg.isNull()) {
        QMessageBox::information(this, "Empty Gallery", "Take some photos first! No recent images found in gallery folder.");
        return;
    }

    // Update timestamp label on viewer
    pff::Image tempImg;
    if (pff::load_pff(lastPhotoPath.toStdString(), tempImg)) {
        std::time_t raw_time = static_cast<std::time_t>(tempImg.header.timestamp);
        std::tm* timeinfo = std::localtime(&raw_time);
        char time_str[64];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        lblViewerTimestamp->setText(QString("CAPTURED: %1  |  FILTER: %2  |  SIZE: %3x%4")
                                   .arg(time_str)
                                   .arg(getFilterName(tempImg.header.filter_type))
                                   .arg(tempImg.header.width)
                                   .arg(tempImg.header.height));
    } else {
        lblViewerTimestamp->setText("CAPTURED: UNKNOWN TIME");
    }

    // Show image scaled to label viewport
    QPixmap pix = QPixmap::fromImage(lastPhotoImg);
    
    // Scale fitting inside the viewer area dynamically
    int maxW = lblViewerImage->parentWidget()->width();
    int maxH = lblViewerImage->parentWidget()->height();
    if (maxW <= 0) maxW = 800;
    if (maxH <= 0) maxH = 500;
    
    lblViewerImage->setPixmap(pix.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Switch screen to index 1 (Viewer Screen)
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onClosePreviewClicked() {
    // Return to Camera view
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::loadLastTakenPhotoThumbnail() {
    QString galleryPath = "/home/zenx/Desktop/My Images";
    QDir dir(galleryPath);
    if (!dir.exists()) {
        return;
    }

    // List PFF files sorted by newest first
    QStringList filters;
    filters << "*.pff";
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time);

    QStringList fileList = dir.entryList();
    if (fileList.isEmpty()) {
        return;
    }

    // Load the newest file
    QString newestFilePath = dir.filePath(fileList.first());
    pff::Image img;
    if (pff::load_pff(newestFilePath.toStdString(), img)) {
        // Convert PFF image data to QImage
        QImage qimg(img.pixels.data(), img.header.width, img.header.height, QImage::Format_RGB888);
        if (!qimg.isNull()) {
            lastPhotoPath = newestFilePath;
            
            // Need to clone the QImage since it points to the memory in 'img' which goes out of scope
            lastPhotoImg = qimg.copy();

            QPixmap thumb = QPixmap::fromImage(lastPhotoImg).scaled(54, 54, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            btnThumbnail->setIcon(QIcon(thumb));
            btnThumbnail->setIconSize(QSize(50, 50));
        }
    }
}

QString MainWindow::getFilterName(int filterId) {
    switch (filterId) {
        case 0: return "Normal";
        case 1: return "Grayscale";
        case 2: return "Sepia";
        case 3: return "Inverted";
        case 4: return "Warm";
        case 5: return "Cool";
        default: return "Custom";
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (stackedWidget->currentIndex() == 1) { // Viewer screen active
        if (event->key() == Qt::Key_Left) {
            navigateViewer(1); // Since list is sorted newest-first, moving +1 goes to older (right-arrow) or prev? Let's make Left go to index-1 (newer)
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Right) {
            navigateViewer(-1); // Right goes to index+1 (older)
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Escape) {
            onClosePreviewClicked();
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::navigateViewer(int direction) {
    QString galleryPath = "/home/zenx/Desktop/My Images";
    QDir dir(galleryPath);
    if (!dir.exists()) return;

    QStringList filters;
    filters << "*.pff";
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time); // newest first

    QStringList fileList = dir.entryList();
    if (fileList.isEmpty()) return;

    // Find current file index
    QFileInfo currentFi(lastPhotoPath);
    QString currentName = currentFi.fileName();
    int index = fileList.indexOf(currentName);
    if (index == -1) {
        index = 0;
    } else {
        index = (index + direction + fileList.size()) % fileList.size();
    }

    QString nextFilePath = dir.filePath(fileList[index]);
    pff::Image tempImg;
    if (pff::load_pff(nextFilePath.toStdString(), tempImg)) {
        QImage qimg(tempImg.pixels.data(), tempImg.header.width, tempImg.header.height, QImage::Format_RGB888);
        if (!qimg.isNull()) {
            lastPhotoPath = nextFilePath;
            lastPhotoImg = qimg.copy();

            std::time_t raw_time = static_cast<std::time_t>(tempImg.header.timestamp);
            std::tm* timeinfo = std::localtime(&raw_time);
            char time_str[64];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            
            lblViewerTimestamp->setText(QString("CAPTURED: %1  |  FILTER: %2  |  SIZE: %3x%4")
                                       .arg(time_str)
                                       .arg(getFilterName(tempImg.header.filter_type))
                                       .arg(tempImg.header.width)
                                       .arg(tempImg.header.height));

            QPixmap pix = QPixmap::fromImage(lastPhotoImg);
            int maxW = lblViewerImage->parentWidget()->width();
            int maxH = lblViewerImage->parentWidget()->height();
            if (maxW <= 0) maxW = 800;
            if (maxH <= 0) maxH = 500;
            
            lblViewerImage->setPixmap(pix.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}


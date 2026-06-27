#include "cameraview.h"
#include <QPainter>
#include <QDateTime>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <cmath>
#include <iostream>

CameraView::CameraView(QWidget *parent)
    : QWidget(parent),
      camera(nullptr),
      captureSession(nullptr),
      videoSink(nullptr),
      animationTick(0),
      isMockMode(true),
      currentFilter(0),
      isFlashActive(false),
      countdownValue(-1) {
    
    // Set standard preview size aspect ratio
    setMinimumSize(480, 360);
    
    // Initialize flash timer
    flashTimer = new QTimer(this);
    flashTimer->setSingleShot(true);
    connect(flashTimer, &QTimer::timeout, this, [this]() {
        isFlashActive = false;
        update();
    });

    // Initialize mock frame timer
    mockTimer = new QTimer(this);
    connect(mockTimer, &QTimer::timeout, this, &CameraView::updateMockFrame);

    // Try starting physical camera
    startCamera();
}

CameraView::~CameraView() {
    stopCamera();
}

void CameraView::setFilterType(int filterType) {
    currentFilter = filterType;
    update();
}

void CameraView::setCountdownValue(int value) {
    countdownValue = value;
    update();
}

void CameraView::startCamera() {
    stopCamera();

    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        std::cout << "No physical camera found. Starting simulated preview mode." << std::endl;
        isMockMode = true;
    } else {
        std::cout << "Physical camera detected: " 
                  << cameras.first().description().toStdString() << std::endl;
        
        camera = new QCamera(cameras.first(), this);
        
        // Find highest resolution format
        QCameraFormat bestFormat;
        for (const QCameraFormat &format : cameras.first().videoFormats()) {
            if (format.resolution().width() > bestFormat.resolution().width()) {
                bestFormat = format;
            }
        }
        if (!bestFormat.resolution().isEmpty()) {
            camera->setCameraFormat(bestFormat);
            std::cout << "Setting camera format to " 
                      << bestFormat.resolution().width() << "x" << bestFormat.resolution().height() << std::endl;
        }

        captureSession = new QMediaCaptureSession(this);
        captureSession->setCamera(camera);
        
        videoSink = new QVideoSink(this);
        captureSession->setVideoSink(videoSink);

        connect(videoSink, &QVideoSink::videoFrameChanged, this, &CameraView::handleVideoFrame);
        
        camera->start();
        isMockMode = false;
    }

    if (isMockMode) {
        mockTimer->start(33); // ~30 FPS
    }
}

void CameraView::stopCamera() {
    mockTimer->stop();
    if (camera) {
        camera->stop();
        delete camera;
        camera = nullptr;
    }
    if (captureSession) {
        delete captureSession;
        captureSession = nullptr;
    }
    videoSink = nullptr; // Deleted by captureSession
}

void CameraView::handleVideoFrame(const QVideoFrame &frame) {
    if (isMockMode) return;

    QVideoFrame cloneFrame(frame);
    if (!cloneFrame.map(QVideoFrame::ReadOnly)) {
        return;
    }

    QImage img = cloneFrame.toImage().convertToFormat(QImage::Format_RGB888);
    cloneFrame.unmap();

    if (img.isNull()) {
        return;
    }

    // Apply active filter
    applyPffFilter(img);

    {
        QMutexLocker locker(&frameMutex);
        currentFrame = img;
    }

    emit frameUpdated();
    update();
}

void CameraView::updateMockFrame() {
    if (!isMockMode) return;

    animationTick = (animationTick + 1) % 360;

    // Create container frame
    QImage img(1920, 1080, QImage::Format_RGB888);
    generateMockFrame(img);
    applyPffFilter(img);

    {
        QMutexLocker locker(&frameMutex);
        currentFrame = img;
    }

    emit frameUpdated();
    update();
}

void CameraView::generateMockFrame(QImage &img) {
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = img.width();
    int h = img.height();

    // 1. Draw a beautiful shifting gradient background (simulating lighting/environment)
    double angle = animationTick * M_PI / 180.0;
    QLinearGradient grad(0, 0, w, h);
    int r1 = static_cast<int>(35 + 20 * std::sin(angle));
    int g1 = static_cast<int>(30 + 15 * std::cos(angle));
    int b1 = static_cast<int>(55 + 25 * std::sin(angle * 1.5));
    
    int r2 = static_cast<int>(15 + 10 * std::cos(angle * 0.8));
    int g2 = static_cast<int>(15 + 10 * std::sin(angle * 1.2));
    int b2 = static_cast<int>(25 + 15 * std::cos(angle));

    grad.setColorAt(0.0, QColor(r1, g1, b1));
    grad.setColorAt(1.0, QColor(r2, g2, b2));
    painter.fillRect(0, 0, w, h, grad);

    // 2. Draw animated grid/star elements
    painter.setPen(QPen(QColor(255, 255, 255, 15), 1));
    for (int x = 40; x < w; x += 40) {
        painter.drawLine(x, 0, x, h);
    }
    for (int y = 40; y < h; y += 40) {
        painter.drawLine(0, y, w, y);
    }

    // 3. Draw a rotating glowing vector shape in the center (representing a subject)
    painter.save();
    painter.translate(w / 2, h / 2);
    painter.rotate(animationTick * 0.5);
    
    QRadialGradient centerGrad(0, 0, 120);
    centerGrad.setColorAt(0.0, QColor(0, 180, 255, 120));
    centerGrad.setColorAt(0.6, QColor(0, 100, 200, 40));
    centerGrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setBrush(centerGrad);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(-120, -120, 240, 240);

    // Draw floating orbit circles
    painter.setPen(QPen(QColor(0, 200, 255, 150), 2, Qt::DashLine));
    painter.drawEllipse(-80, -80, 160, 160);

    painter.rotate(-animationTick * 1.2);
    painter.setPen(QPen(QColor(255, 165, 0, 120), 1.5));
    painter.drawRect(-40, -40, 80, 80);
    painter.restore();

    // 4. Draw HUD view finder lines
    painter.setPen(QPen(QColor(255, 255, 255, 180), 2));
    int margin = 30;
    int len = 25;
    // Top-Left corner
    painter.drawLine(margin, margin, margin + len, margin);
    painter.drawLine(margin, margin, margin, margin + len);
    // Top-Right corner
    painter.drawLine(w - margin, margin, w - margin - len, margin);
    painter.drawLine(w - margin, margin, w - margin, margin + len);
    // Bottom-Left corner
    painter.drawLine(margin, h - margin, margin + len, h - margin);
    painter.drawLine(margin, h - margin, margin, h - margin - len);
    // Bottom-Right corner
    painter.drawLine(w - margin, h - margin, w - margin - len, h - margin);
    painter.drawLine(w - margin, h - margin, w - margin, h - margin - len);

    // Center crosshair
    painter.setPen(QPen(QColor(255, 255, 255, 100), 1));
    painter.drawLine(w / 2 - 10, h / 2, w / 2 + 10, h / 2);
    painter.drawLine(w / 2, h / 2 - 10, w / 2, h / 2 + 10);

    // 5. Draw active scanner sweep line
    int sweepY = static_cast<int>((h / 2) + (h / 2 - 40) * std::sin(animationTick * M_PI / 90.0));
    QLinearGradient sweepGrad(0, sweepY - 4, 0, sweepY + 4);
    sweepGrad.setColorAt(0.0, QColor(0, 255, 200, 0));
    sweepGrad.setColorAt(0.5, QColor(0, 255, 200, 100));
    sweepGrad.setColorAt(1.0, QColor(0, 255, 200, 0));
    painter.fillRect(margin, sweepY - 4, w - 2 * margin, 8, sweepGrad);

    // 6. HUD Info text overlay
    painter.setPen(QColor(255, 255, 255, 220));
    QFont font = painter.font();
    font.setFamily("monospace");
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    painter.drawText(margin + 10, margin + 20, "REC ●");
    
    // Draw battery indicator
    painter.drawText(w - margin - 120, margin + 20, "BATTERY: 98%");
    painter.drawText(margin + 10, h - margin - 10, "1920x1080 @ 30FPS");
    painter.drawText(w - margin - 140, h - margin - 10, "FORMAT: PFF (RLE)");

    // Draw live timestamp
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    painter.drawText(w / 2 - 70, margin + 20, timeStr);
}

void CameraView::applyPffFilter(QImage &img) {
    if (currentFilter == 0) return; // Normal, do nothing

    int w = img.width();
    int h = img.height();
    
    for (int y = 0; y < h; ++y) {
        uchar *line = img.scanLine(y);
        for (int x = 0; x < w; ++x) {
            uchar *p = &line[x * 3];
            uchar r = p[0];
            uchar g = p[1];
            uchar b = p[2];

            switch (currentFilter) {
                case 1: { // Grayscale
                    uchar gray = static_cast<uchar>(0.299f * r + 0.587f * g + 0.114f * b);
                    p[0] = gray;
                    p[1] = gray;
                    p[2] = gray;
                    break;
                }
                case 2: { // Sepia
                    int tr = static_cast<int>(0.393f * r + 0.769f * g + 0.189f * b);
                    int tg = static_cast<int>(0.349f * r + 0.686f * g + 0.168f * b);
                    int tb = static_cast<int>(0.272f * r + 0.534f * g + 0.131f * b);
                    p[0] = static_cast<uchar>(std::min(tr, 255));
                    p[1] = static_cast<uchar>(std::min(tg, 255));
                    p[2] = static_cast<uchar>(std::min(tb, 255));
                    break;
                }
                case 3: { // Invert
                    p[0] = 255 - r;
                    p[1] = 255 - g;
                    p[2] = 255 - b;
                    break;
                }
                case 4: { // Warm
                    int tr = r + 30;
                    int tg = g + 15;
                    p[0] = static_cast<uchar>(std::min(tr, 255));
                    p[1] = static_cast<uchar>(std::min(tg, 255));
                    break;
                }
                case 5: { // Cool
                    int tb = b + 35;
                    int tg = g + 10;
                    p[2] = static_cast<uchar>(std::min(tb, 255));
                    p[1] = static_cast<uchar>(std::min(tg, 255));
                    break;
                }
                default:
                    break;
            }
        }
    }
}

QImage CameraView::captureCurrentFrame() {
    QMutexLocker locker(&frameMutex);
    if (currentFrame.isNull()) {
        // Return dummy black frame if null
        QImage dummy(640, 480, QImage::Format_RGB888);
        dummy.fill(Qt::black);
        return dummy;
    }
    return currentFrame.copy();
}

void CameraView::triggerFlash() {
    isFlashActive = true;
    update();
    flashTimer->start(120); // flash for 120 ms
}

void CameraView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    {
        QMutexLocker locker(&frameMutex);
        if (!currentFrame.isNull()) {
            // Draw scaling maintaining aspect ratio
            QImage scaled = currentFrame.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int dx = (w - scaled.width()) / 2;
            int dy = (h - scaled.height()) / 2;
            painter.drawImage(dx, dy, scaled);

            // Fill borders with dark background if aspect ratios differ
            painter.setBrush(QColor(15, 15, 17));
            painter.setPen(Qt::NoPen);
            if (dx > 0) {
                painter.drawRect(0, 0, dx, h);
                painter.drawRect(w - dx, 0, dx, h);
            }
            if (dy > 0) {
                painter.drawRect(0, 0, w, dy);
                painter.drawRect(0, h - dy, w, dy);
            }
        } else {
            // Loading state
            painter.fillRect(0, 0, w, h, QColor(20, 20, 22));
            painter.setPen(QColor(120, 120, 125));
            painter.drawText(rect(), Qt::AlignCenter, "INITIALIZING CAMERA VIEW...");
        }
    }

    // Draw shutter flash overlay if active
    if (isFlashActive) {
        painter.fillRect(0, 0, w, h, QColor(255, 255, 255, 235));
    }

    // Draw countdown overlay if active
    if (countdownValue > 0) {
        painter.save();
        QFont font = painter.font();
        font.setFamily("sans-serif");
        font.setPointSize(96);
        font.setBold(true);
        painter.setFont(font);

        QString countStr = QString::number(countdownValue);
        QFontMetrics fm(font);
        int textW = fm.horizontalAdvance(countStr);
        int textH = fm.height();
        int tx = (w - textW) / 2;
        int ty = (h + fm.ascent() - fm.descent()) / 2;

        // Shadow/Border (draw outline)
        painter.setPen(QColor(0, 0, 0, 200));
        for (int dx = -3; dx <= 3; ++dx) {
            for (int dy = -3; dy <= 3; ++dy) {
                if (dx != 0 || dy != 0) {
                    painter.drawText(tx + dx, ty + dy, countStr);
                }
            }
        }

        // White Text
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(tx, ty, countStr);
        painter.restore();
    }
}

void CameraView::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}

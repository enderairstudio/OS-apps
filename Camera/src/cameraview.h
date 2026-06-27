#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QMutex>

QT_BEGIN_NAMESPACE
class QCamera;
class QMediaCaptureSession;
class QVideoSink;
class QVideoFrame;
QT_END_NAMESPACE

class CameraView : public QWidget {
    Q_OBJECT

public:
    explicit CameraView(QWidget *parent = nullptr);
    ~CameraView() override;

    void setFilterType(int filterType);
    void setCountdownValue(int value);
    void startCamera();
    void stopCamera();
    
    // Returns a copy of the current frame (filtered)
    QImage captureCurrentFrame();

    // Trigger flash animation
    void triggerFlash();

signals:
    void frameUpdated();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateMockFrame();
    void handleVideoFrame(const QVideoFrame &frame);

private:
    void initCamera();
    void generateMockFrame(QImage &img);
    void applyPffFilter(QImage &img);

    // Qt6 Multimedia variables
    QCamera *camera;
    QMediaCaptureSession *captureSession;
    QVideoSink *videoSink;

    // Drawing variables
    QImage currentFrame;
    QMutex frameMutex;

    // Simulation variables
    QTimer *mockTimer;
    int animationTick;
    bool isMockMode;
    int currentFilter;

    // Flash effect variables
    bool isFlashActive;
    QTimer *flashTimer;
    int countdownValue;
};

#endif // CAMERAVIEW_H

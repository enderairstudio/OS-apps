#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "cameraview.h"
#include "settingsmenu.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSettingsClicked();
    void onShutterClicked();
    void onGalleryClicked();
    void onClosePreviewClicked();
    
    // Timer countdown slot
    void onCountdownTick();

private:
    void setupUi();
    void loadLastTakenPhotoThumbnail();
    void capturePhoto();
    
    // Custom filter name formatter
    QString getFilterName(int filterId);

    // UI elements
    QStackedWidget *stackedWidget;
    
    // Camera Screen widgets
    QWidget *cameraScreen;
    CameraView *cameraView;
    QPushButton *btnSettings;
    QPushButton *btnShutter;
    QPushButton *btnThumbnail;

    // Image Viewer Screen widgets
    QWidget *viewerScreen;
    QLabel *lblViewerImage;
    QLabel *lblViewerTimestamp;
    QPushButton *btnCloseViewer;

    // Settings popup
    SettingsMenu *settingsMenu;

    // Countdown variables
    QTimer *countdownTimer;
    int remainingSeconds;
    
    // Path of the last taken photo
    QString lastPhotoPath;
    QImage lastPhotoImg;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void navigateViewer(int direction);
};

#endif // MAINWINDOW_H

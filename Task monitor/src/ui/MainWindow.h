#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../core/SystemInfo.h"
#include "CircularProgressBar.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateStats();

private:
    SystemInfo systemInfo;
    QTimer *updateTimer;

    // UI elements
    // Box 1 (CPU/GPU)
    CircularProgressBar *cpuProgress;
    CircularProgressBar *gpuProgress;

    // Box 2 (Memory/Swap)
    CircularProgressBar *memoryProgress;
    CircularProgressBar *swapProgress;

    // Device Info Bar labels
    QLabel *cpuModelLabel;
    QLabel *gpuModelLabel;
    QLabel *ramModelLabel;
    QLabel *motherboardLabel;
    QLabel *sysModelLabel;

    // Box 3 (Disks)
    QLabel *diskTitleLabel;
    QLabel *diskInfoLabel;
    QProgressBar *diskUsageBar;

    // Box 4 (Networks)
    QLabel *netTitleLabel;
    QLabel *wifiNameLabel;
    QLabel *ipLabel;
    QLabel *downSpeedLabel;
    QLabel *upSpeedLabel;

    void setupUi();
    QFrame* createBox1();
    QFrame* createBox2();
    QFrame* createDeviceInfoBar();
    QFrame* createBox3();
    QFrame* createBox4();
};

#endif // MAINWINDOW_H

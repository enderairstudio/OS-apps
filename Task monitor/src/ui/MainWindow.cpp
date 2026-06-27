#include "MainWindow.h"
#include <QIcon>
#include <QPixmap>
#include <QGridLayout>
#include <QGroupBox>
#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();

    // Setup update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateStats);
    updateTimer->start(1000); // 1 second interval

    // Initial update
    updateStats();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    // Set window properties
    setWindowTitle("CoreVision Task Monitor");
    setMinimumSize(960, 680);
    
    // Set application icon from logo
    setWindowIcon(QIcon(":/logo.jpg"));

    // Modern cyber dark stylesheet (Sleek Gray Theme)
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #202225;"
        "}"
        "QLabel {"
        "    color: #ffffff;"
        "    font-family: 'Outfit', 'Segoe UI', sans-serif;"
        "}"
        "QFrame#Card {"
        "    background-color: #2f3136;"
        "    border: 1px solid #4f545c;"
        "    border-radius: 16px;"
        "}"
        "QFrame#Card:hover {"
        "    border: 1px solid #b9bbbe;"
        "}"
        "QFrame#DevBar {"
        "    background-color: #2f3136;"
        "    border: 1px solid #4f545c;"
        "    border-radius: 12px;"
        "}"
        "QProgressBar {"
        "    border: 1px solid #4f545c;"
        "    border-radius: 6px;"
        "    background-color: #202225;"
        "    height: 16px;"
        "    text-align: center;"
        "    color: transparent;"
        "}"
        "QProgressBar::chunk {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #72767d, stop:1 #e3e5e8);"
        "    border-radius: 5px;"
        "}"
    );

    // Main central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Header bar (Title)
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel("Task Monitor", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #e1e7f0; letter-spacing: 2px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    mainLayout->addLayout(headerLayout);

    // Top Row: Box 1 (CPU/GPU) & Box 2 (Memory/Swap)
    QHBoxLayout *topRowLayout = new QHBoxLayout();
    topRowLayout->setSpacing(20);

    QFrame *box1 = createBox1();
    QFrame *box2 = createBox2();
    
    topRowLayout->addWidget(box1, 1);
    topRowLayout->addWidget(box2, 1);
    
    mainLayout->addLayout(topRowLayout, 4);

    // Middle Row: Device Info Bar
    QFrame *devInfoBar = createDeviceInfoBar();
    mainLayout->addWidget(devInfoBar, 1);

    // Bottom Row: Box 3 (Disks) & Box 4 (Networks)
    QHBoxLayout *bottomRowLayout = new QHBoxLayout();
    bottomRowLayout->setSpacing(20);

    QFrame *box3 = createBox3();
    QFrame *box4 = createBox4();

    bottomRowLayout->addWidget(box3, 1);
    bottomRowLayout->addWidget(box4, 1);

    mainLayout->addLayout(bottomRowLayout, 3);
}

QFrame* MainWindow::createBox1() {
    QFrame *frame = new QFrame(this);
    frame->setObjectName("Card");
    
    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Left side: GPU
    QVBoxLayout *gpuLayout = new QVBoxLayout();
    gpuLayout->setSpacing(10);
    QLabel *gpuTitle = new QLabel("GPU", this);
    gpuTitle->setAlignment(Qt::AlignCenter);
    gpuTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #b9bbbe; letter-spacing: 1px;");
    gpuProgress = new CircularProgressBar(this);
    gpuProgress->setColor(QColor(185, 187, 190)); // Slate gray color
    gpuLayout->addWidget(gpuTitle);
    gpuLayout->addWidget(gpuProgress, 1);

    // Middle separator
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setStyleSheet("background-color: #4f545c; min-width: 1px; max-width: 1px; border: none;");

    // Right side: CPU
    QVBoxLayout *cpuLayout = new QVBoxLayout();
    cpuLayout->setSpacing(10);
    QLabel *cpuTitle = new QLabel("CPU", this);
    cpuTitle->setAlignment(Qt::AlignCenter);
    cpuTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #b9bbbe; letter-spacing: 1px;");
    cpuProgress = new CircularProgressBar(this);
    cpuProgress->setColor(QColor(185, 187, 190)); // Slate gray color
    cpuLayout->addWidget(cpuTitle);
    cpuLayout->addWidget(cpuProgress, 1);

    layout->addLayout(gpuLayout, 1);
    layout->addWidget(separator);
    layout->addLayout(cpuLayout, 1);

    return frame;
}

QFrame* MainWindow::createBox2() {
    QFrame *frame = new QFrame(this);
    frame->setObjectName("Card");

    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Left side: Swap
    QVBoxLayout *swapLayout = new QVBoxLayout();
    swapLayout->setSpacing(10);
    QLabel *swapTitle = new QLabel("Swap", this);
    swapTitle->setAlignment(Qt::AlignCenter);
    swapTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #b9bbbe; letter-spacing: 1px;");
    swapProgress = new CircularProgressBar(this);
    swapProgress->setColor(QColor(185, 187, 190)); // Slate gray color
    swapLayout->addWidget(swapTitle);
    swapLayout->addWidget(swapProgress, 1);

    // Middle separator
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setStyleSheet("background-color: #4f545c; min-width: 1px; max-width: 1px; border: none;");

    // Right side: Memory (RAM)
    QVBoxLayout *memoryLayout = new QVBoxLayout();
    memoryLayout->setSpacing(10);
    QLabel *memTitle = new QLabel("Memory", this);
    memTitle->setAlignment(Qt::AlignCenter);
    memTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #b9bbbe; letter-spacing: 1px;");
    memoryProgress = new CircularProgressBar(this);
    memoryProgress->setColor(QColor(185, 187, 190)); // Slate gray color
    memoryLayout->addWidget(memTitle);
    memoryLayout->addWidget(memoryProgress, 1);

    layout->addLayout(swapLayout, 1);
    layout->addWidget(separator);
    layout->addLayout(memoryLayout, 1);

    return frame;
}

QFrame* MainWindow::createDeviceInfoBar() {
    QFrame *frame = new QFrame(this);
    frame->setObjectName("DevBar");
    
    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(15, 10, 15, 10);
    layout->setSpacing(10);

    // Retrieve initial hardware details
    HardwareInfo hw = systemInfo.getHardwareInfo();

    // CPU Info Badge
    QVBoxLayout *cpuCol = new QVBoxLayout();
    QLabel *cpuHeader = new QLabel("PROCESSOR", this);
    cpuHeader->setStyleSheet("font-size: 9px; font-weight: bold; color: #8795a7; letter-spacing: 1px;");
    cpuModelLabel = new QLabel(hw.cpuModel, this);
    cpuModelLabel->setStyleSheet("font-size: 12px; font-weight: bold;");
    cpuCol->addWidget(cpuHeader);
    cpuCol->addWidget(cpuModelLabel);

    // GPU Info Badge
    QVBoxLayout *gpuCol = new QVBoxLayout();
    QLabel *gpuHeader = new QLabel("GRAPHICS", this);
    gpuHeader->setStyleSheet("font-size: 9px; font-weight: bold; color: #8795a7; letter-spacing: 1px;");
    gpuModelLabel = new QLabel(hw.gpuModel, this);
    gpuModelLabel->setStyleSheet("font-size: 12px; font-weight: bold;");
    gpuCol->addWidget(gpuHeader);
    gpuCol->addWidget(gpuModelLabel);

    // RAM Info Badge
    QVBoxLayout *ramCol = new QVBoxLayout();
    QLabel *ramHeader = new QLabel("SYSTEM MEMORY", this);
    ramHeader->setStyleSheet("font-size: 9px; font-weight: bold; color: #8795a7; letter-spacing: 1px;");
    ramModelLabel = new QLabel(hw.ramSize, this);
    ramModelLabel->setStyleSheet("font-size: 12px; font-weight: bold;");
    ramCol->addWidget(ramHeader);
    ramCol->addWidget(ramModelLabel);

    // Motherboard Info Badge
    QVBoxLayout *mbCol = new QVBoxLayout();
    QLabel *mbHeader = new QLabel("MOTHERBOARD", this);
    mbHeader->setStyleSheet("font-size: 9px; font-weight: bold; color: #8795a7; letter-spacing: 1px;");
    motherboardLabel = new QLabel(hw.motherboard, this);
    motherboardLabel->setStyleSheet("font-size: 12px; font-weight: bold;");
    mbCol->addWidget(mbHeader);
    mbCol->addWidget(motherboardLabel);

    // Product Model Badge
    QVBoxLayout *sysCol = new QVBoxLayout();
    QLabel *sysHeader = new QLabel("SYSTEM MODEL", this);
    sysHeader->setStyleSheet("font-size: 9px; font-weight: bold; color: #8795a7; letter-spacing: 1px;");
    sysModelLabel = new QLabel(hw.sysModel, this);
    sysModelLabel->setStyleSheet("font-size: 12px; font-weight: bold;");
    sysCol->addWidget(sysHeader);
    sysCol->addWidget(sysModelLabel);

    layout->addLayout(cpuCol, 3);
    layout->addLayout(gpuCol, 3);
    layout->addLayout(ramCol, 2);
    layout->addLayout(mbCol, 3);
    layout->addLayout(sysCol, 3);

    return frame;
}

QFrame* MainWindow::createBox3() {
    QFrame *frame = new QFrame(this);
    frame->setObjectName("Card");

    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(8);

    diskTitleLabel = new QLabel("Disks", this);
    diskTitleLabel->setAlignment(Qt::AlignCenter);
    diskTitleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #8795a7; letter-spacing: 1px;");
    layout->addWidget(diskTitleLabel);

    layout->addStretch();

    diskInfoLabel = new QLabel("Loading space...", this);
    diskInfoLabel->setAlignment(Qt::AlignCenter);
    diskInfoLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #e1e7f0;");
    layout->addWidget(diskInfoLabel);

    diskUsageBar = new QProgressBar(this);
    diskUsageBar->setValue(0);
    layout->addWidget(diskUsageBar);

    layout->addStretch();

    return frame;
}

QFrame* MainWindow::createBox4() {
    QFrame *frame = new QFrame(this);
    frame->setObjectName("Card");

    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(6);

    netTitleLabel = new QLabel("Networks", this);
    netTitleLabel->setAlignment(Qt::AlignCenter);
    netTitleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #8795a7; letter-spacing: 1px;");
    layout->addWidget(netTitleLabel);

    layout->addStretch();

    // Wifi Name
    QHBoxLayout *wifiRow = new QHBoxLayout();
    wifiRow->addStretch();
    QLabel *wifiIcon = new QLabel("📶", this);
    wifiIcon->setStyleSheet("font-size: 13px;");
    wifiNameLabel = new QLabel("Searching wifi...", this);
    wifiNameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #e1e7f0;");
    wifiRow->addWidget(wifiIcon);
    wifiRow->addWidget(wifiNameLabel);
    wifiRow->addStretch();
    layout->addLayout(wifiRow);

    // IPv4 Address
    ipLabel = new QLabel("IP: --", this);
    ipLabel->setAlignment(Qt::AlignCenter);
    ipLabel->setStyleSheet("font-size: 11px; color: #78859e; font-family: monospace;");
    layout->addWidget(ipLabel);

    // Speed display
    QHBoxLayout *speedLayout = new QHBoxLayout();
    speedLayout->setSpacing(20);
    speedLayout->addStretch();

    downSpeedLabel = new QLabel("↓ 0 B/s", this);
    downSpeedLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #e1e7f0;");
    
    upSpeedLabel = new QLabel("↑ 0 B/s", this);
    upSpeedLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #8795a7;");

    speedLayout->addWidget(downSpeedLabel);
    speedLayout->addWidget(upSpeedLabel);
    speedLayout->addStretch();

    layout->addLayout(speedLayout);
    layout->addStretch();

    return frame;
}

void MainWindow::updateStats() {
    // 1. Tick the monitor engine
    systemInfo.update();

    // 2. Fetch and apply metrics
    double cpuUse = systemInfo.getCpuUsage();
    cpuProgress->setValue(cpuUse);
    cpuProgress->setCustomText(QString("%1%").arg(qRound(cpuUse)));

    double gpuUse = systemInfo.getGpuUsage();
    gpuProgress->setValue(gpuUse);
    gpuProgress->setCustomText(QString("%1%").arg(qRound(gpuUse)));

    // RAM
    double memUsed, memTotal;
    systemInfo.getMemoryUsage(memUsed, memTotal);
    double ramPercent = (memTotal > 0) ? (memUsed / memTotal) * 100.0 : 0.0;
    memoryProgress->setValue(ramPercent);
    memoryProgress->setCustomText(
        QString("%1").arg(memUsed, 0, 'f', 1),
        QString("%1 GiB").arg(memTotal, 0, 'f', 1)
    );

    // Swap
    double swapUsedBytes, swapTotalGiB;
    systemInfo.getSwapUsage(swapUsedBytes, swapTotalGiB);
    double swapUsedGiB = swapUsedBytes / (1024.0 * 1024.0 * 1024.0);
    double swapPercent = (swapTotalGiB > 0) ? (swapUsedGiB / swapTotalGiB) * 100.0 : 0.0;
    swapProgress->setValue(swapPercent);
    
    // Format swap used string (e.g. B, KB, MB, GiB)
    QString swapUsedStr;
    if (swapUsedBytes < 1024.0) {
        swapUsedStr = QString("%1 B").arg(swapUsedBytes, 0, 'f', 0);
    } else if (swapUsedBytes < 1024.0 * 1024.0) {
        swapUsedStr = QString("%1 KB").arg(swapUsedBytes / 1024.0, 0, 'f', 1);
    } else if (swapUsedBytes < 1024.0 * 1024.0 * 1024.0) {
        swapUsedStr = QString("%1 MB").arg(swapUsedBytes / (1024.0 * 1024.0), 0, 'f', 1);
    } else {
        swapUsedStr = QString("%1 GiB").arg(swapUsedGiB, 0, 'f', 1);
    }
    swapProgress->setCustomText(
        swapUsedStr,
        QString("%1 GiB").arg(swapTotalGiB, 0, 'f', 1)
    );

    // Disk
    QVector<DiskInfo> disks = systemInfo.getDiskInfo();
    if (!disks.isEmpty()) {
        DiskInfo d = disks[0]; // Primary disk (root)
        diskUsageBar->setValue(qRound(d.percentage));
        diskInfoLabel->setText(QString("%1: %2 / %3 GiB")
                               .arg(d.mountPoint)
                               .arg(QString::number(d.usedGiB, 'f', 1))
                               .arg(QString::number(d.totalGiB, 'f', 1)));
    }

    // Network
    NetworkInfo net = systemInfo.getNetworkInfo();
    wifiNameLabel->setText(net.ssid);
    ipLabel->setText("IP Address: " + net.ipv4);

    // Format speed strings
    auto formatSpeed = [](double bytesPerSec) -> QString {
        if (bytesPerSec < 1024.0) {
            return QString("%1 B/s").arg(bytesPerSec, 0, 'f', 0);
        } else if (bytesPerSec < 1024.0 * 1024.0) {
            return QString("%1 KB/s").arg(bytesPerSec / 1024.0, 0, 'f', 1);
        } else {
            return QString("%1 MB/s").arg(bytesPerSec / (1024.0 * 1024.0), 0, 'f', 1);
        }
    };
    downSpeedLabel->setText("↓ " + formatSpeed(net.downloadSpeed));
    upSpeedLabel->setText("↑ " + formatSpeed(net.uploadSpeed));
}

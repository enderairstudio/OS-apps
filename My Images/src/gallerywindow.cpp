#include "gallerywindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QDateTime>
#include <QKeyEvent>
#include <QPixmap>
#include <QFrame>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <iostream>
#include "pff.h"

// ============================================================================
// GALLERY ITEM IMPLEMENTATION
// ============================================================================

GalleryItem::GalleryItem(const QString &filepath, const QPixmap &thumb, const QString &filename, QWidget *parent)
    : QFrame(parent), m_filepath(filepath), m_selected(false) {
    
    setFixedSize(220, 200);
    
    // Style layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    // Frame styling
    setStyleSheet(
        "QFrame {"
        "  background-color: #ffffff;"
        "  border: 1px solid #e9ecef;"
        "  border-radius: 10px;"
        "}"
    );

    // Thumbnail Label
    lblThumb = new QLabel(this);
    lblThumb->setFixedSize(200, 150);
    lblThumb->setPixmap(thumb);
    lblThumb->setStyleSheet("border-radius: 6px; border: none; background: #f1f3f5;");
    layout->addWidget(lblThumb);

    // Text Label
    lblText = new QLabel(filename, this);
    lblText->setAlignment(Qt::AlignCenter);
    lblText->setStyleSheet("color: #495057; font-size: 11px; font-weight: 600; border: none; background: transparent;");
    layout->addWidget(lblText);

    // Selection Indicator Overlay (checkmark in top right)
    overlayCheck = new QFrame(this);
    overlayCheck->setFixedSize(24, 24);
    overlayCheck->move(185, 15);
    overlayCheck->setStyleSheet(
        "QFrame {"
        "  background-color: #0d6efd;"
        "  border: 2px solid #ffffff;"
        "  border-radius: 12px;"
        "}"
    );
    
    QLabel *lblCheck = new QLabel("✓", overlayCheck);
    lblCheck->setAlignment(Qt::AlignCenter);
    lblCheck->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: bold; border: none; background: transparent;");
    QVBoxLayout *checkLayout = new QVBoxLayout(overlayCheck);
    checkLayout->setContentsMargins(0, 0, 0, 0);
    checkLayout->addWidget(lblCheck);
    
    overlayCheck->setVisible(false);

    // Setup shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);
}

void GalleryItem::setSelectedState(bool selected) {
    m_selected = selected;
    overlayCheck->setVisible(selected);
    
    if (selected) {
        setStyleSheet(
            "QFrame {"
            "  background-color: #f1f8ff;"
            "  border: 2px solid #0d6efd;"
            "  border-radius: 10px;"
            "}"
        );
    } else {
        setStyleSheet(
            "QFrame {"
            "  background-color: #ffffff;"
            "  border: 1px solid #e9ecef;"
            "  border-radius: 10px;"
            "}"
        );
    }
}

void GalleryItem::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        bool ctrlHeld = (event->modifiers() & Qt::ControlModifier);
        emit itemPressed(m_filepath, ctrlHeld);
    }
}

void GalleryItem::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit itemDoubleClicked(m_filepath);
    }
}


QIcon createVectorIcon(const QString &type, const QColor &color) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    if (type == "photos") {
        painter.drawRoundedRect(3, 7, 18, 14, 2, 2);
        painter.drawEllipse(10, 12, 4, 4);
        painter.setBrush(color);
        painter.drawRect(10, 5, 4, 2);
    } else if (type == "videos") {
        painter.drawRoundedRect(3, 6, 12, 12, 2, 2);
        QPolygon poly;
        poly << QPoint(15, 9) << QPoint(21, 5) << QPoint(21, 19) << QPoint(15, 15);
        painter.setBrush(color);
        painter.drawPolygon(poly);
    } else if (type == "trash") {
        painter.drawRoundedRect(5, 8, 14, 14, 1, 1);
        painter.drawLine(3, 6, 21, 6);
        painter.drawPolyline(QPolygon() << QPoint(9, 6) << QPoint(9, 3) << QPoint(15, 3) << QPoint(15, 6));
        painter.drawLine(9, 10, 9, 18);
        painter.drawLine(12, 10, 12, 18);
        painter.drawLine(15, 10, 15, 18);
    }
    return QIcon(pixmap);
}

void GalleryWindow::updateSidebarIcons() {
    QColor activeColor("#1a73e8");
    QColor inactiveColor("#5f6368");

    btnTabPhotos->setIcon(createVectorIcon("photos", (currentTab == TAB_PHOTOS) ? activeColor : inactiveColor));
    btnTabVideos->setIcon(createVectorIcon("videos", (currentTab == TAB_VIDEOS) ? activeColor : inactiveColor));
    btnTabTrash->setIcon(createVectorIcon("trash", (currentTab == TAB_TRASH) ? activeColor : inactiveColor));
}

GalleryWindow::GalleryWindow(QWidget *parent)
    : QMainWindow(parent), currentTab(TAB_PHOTOS) {
    
    setWindowTitle("Photo Gallery - PFF Viewer");
    resize(1080, 720);

    // General styling
    setStyleSheet(
        "QMainWindow {"
        "  background-color: #ffffff;"
        "}"
        "QWidget {"
        "  color: #212529;"
        "  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;"
        "}"
        "QScrollArea {"
        "  border: none;"
        "  background-color: #ffffff;"
        "}"
        "QScrollBar:vertical {"
        "  border: none;"
        "  background: #f1f3f5;"
        "  width: 10px;"
        "  margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #ced4da;"
        "  border-radius: 5px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #adb5bd;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );

    setupUi();

    // Directory monitoring for real-time update
    watcher = new QFileSystemWatcher(this);
    QString galleryPath = "/home/zenx/Desktop/My Images";
    QDir().mkpath(galleryPath);
    
    // Ensure trash folder exists
    QDir().mkpath(galleryPath + "/.trash");
    
    watcher->addPath(galleryPath);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &GalleryWindow::refreshGallery);

    // Initial load
    refreshGallery();
}

GalleryWindow::~GalleryWindow() {
}

void GalleryWindow::setupUi() {
    // Main layouts
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // SIDEBAR NAVIGATION (Theme: White sidebar)
    // ==========================================
    leftSidebar = new QWidget(centralWidget);
    leftSidebar->setFixedWidth(220);
    leftSidebar->setStyleSheet(
        "QWidget {"
        "  background-color: #f8f9fa;"
        "  border-right: 1px solid #dee2e6;"
        "}"
    );
    
    QVBoxLayout *sideLayout = new QVBoxLayout(leftSidebar);
    sideLayout->setContentsMargins(16, 24, 16, 24);
    sideLayout->setSpacing(8);

    // Sidebar logo / Title
    QLabel *lblLogo = new QLabel("Antigravity OS", leftSidebar);
    lblLogo->setStyleSheet("font-size: 11px; font-weight: 800; color: #868e96; letter-spacing: 1.5px; margin-bottom: 2px; border: none; background: transparent;");
    sideLayout->addWidget(lblLogo);
    
    QLabel *lblAppName = new QLabel("Media Suite", leftSidebar);
    lblAppName->setStyleSheet("font-size: 20px; font-weight: 800; color: #212529; margin-bottom: 24px; border: none; background: transparent;");
    sideLayout->addWidget(lblAppName);

    auto styleTabButton = [](QPushButton *btn) {
        btn->setCheckable(true);
        btn->setFlat(true);
        btn->setStyleSheet(
            "QPushButton {"
            "  text-align: left;"
            "  padding: 10px 16px;"
            "  font-size: 14px;"
            "  font-weight: 600;"
            "  color: #495057;"
            "  background-color: transparent;"
            "  border-radius: 8px;"
            "  border: none;"
            "}"
            "QPushButton:hover {"
            "  background-color: #e9ecef;"
            "  color: #212529;"
            "}"
            "QPushButton:checked {"
            "  background-color: #e8f0fe;"
            "  color: #1a73e8;"
            "  font-weight: bold;"
            "}"
        );
    };

    btnTabPhotos = new QPushButton("📸   Photos", leftSidebar);
    styleTabButton(btnTabPhotos);
    btnTabPhotos->setChecked(true);
    connect(btnTabPhotos, &QPushButton::clicked, this, &GalleryWindow::selectPhotosTab);
    sideLayout->addWidget(btnTabPhotos);

    btnTabVideos = new QPushButton("🎥   Videos", leftSidebar);
    styleTabButton(btnTabVideos);
    connect(btnTabVideos, &QPushButton::clicked, this, &GalleryWindow::selectVideosTab);
    sideLayout->addWidget(btnTabVideos);

    btnTabTrash = new QPushButton("🗑️   Trash", leftSidebar);
    styleTabButton(btnTabTrash);
    connect(btnTabTrash, &QPushButton::clicked, this, &GalleryWindow::selectTrashTab);
    sideLayout->addWidget(btnTabTrash);

    sideLayout->addStretch();
    mainLayout->addWidget(leftSidebar);

    // ==========================================
    // STACKED WIDGET (Grid View / Detailed View)
    // ==========================================
    stackedWidget = new QStackedWidget(centralWidget);
    mainLayout->addWidget(stackedWidget, 1);

    // ==========================================
    // SCREEN 1: GRID VIEW SCREEN
    // ==========================================
    gridScreen = new QWidget(stackedWidget);
    QVBoxLayout *gridScreenLayout = new QVBoxLayout(gridScreen);
    gridScreenLayout->setContentsMargins(0, 0, 0, 0);
    gridScreenLayout->setSpacing(0);

    // Header bar
    QWidget *header = new QWidget(gridScreen);
    header->setFixedHeight(70);
    header->setStyleSheet(
        "QWidget {"
        "  background-color: #ffffff;"
        "  border-bottom: 1px solid #f1f3f5;"
        "}"
    );
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    lblTitle = new QLabel("PHOTOS", header);
    lblTitle->setStyleSheet("color: #1a1d20; font-size: 20px; font-weight: 800; letter-spacing: 0.5px;");
    headerLayout->addWidget(lblTitle);
    
    headerLayout->addStretch();

    btnRefresh = new QPushButton("Refresh", header);
    btnRefresh->setStyleSheet(
        "QPushButton {"
        "  color: #495057;"
        "  background-color: #ffffff;"
        "  border: 1px solid #ced4da;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  color: #0b5ed7;"
        "  background-color: #f8f9fa;"
        "  border-color: #86b7fe;"
        "}"
    );
    connect(btnRefresh, &QPushButton::clicked, this, &GalleryWindow::refreshGallery);
    headerLayout->addWidget(btnRefresh);

    gridScreenLayout->addWidget(header);

    // Multi-Selection Control Bar (Top banner that slides down/shows when items are selected)
    selectionControlBar = new QWidget(gridScreen);
    selectionControlBar->setFixedHeight(50);
    selectionControlBar->setStyleSheet("background-color: #e8f0fe; border-bottom: 1px solid #d2e3fc;");
    QHBoxLayout *selLayout = new QHBoxLayout(selectionControlBar);
    selLayout->setContentsMargins(24, 0, 24, 0);

    lblSelectionCount = new QLabel("0 items selected", selectionControlBar);
    lblSelectionCount->setStyleSheet("color: #1967d2; font-size: 14px; font-weight: bold;");
    selLayout->addWidget(lblSelectionCount);
    selLayout->addStretch();

    // Action button styling helpers
    auto styleActionBtn = [](QPushButton* btn, const QString& normalColor, const QString& hoverColor) {
        btn->setStyleSheet(
            QString(
                "QPushButton {"
                "  color: #ffffff;"
                "  background-color: %1;"
                "  border: none;"
                "  border-radius: 4px;"
                "  padding: 6px 14px;"
                "  font-size: 12px;"
                "  font-weight: bold;"
                "}"
                "QPushButton:hover { background-color: %2; }"
            ).arg(normalColor, hoverColor)
        );
    };

    btnRestore = new QPushButton("Restore", selectionControlBar);
    styleActionBtn(btnRestore, "#198754", "#157347"); // Green
    connect(btnRestore, &QPushButton::clicked, this, &GalleryWindow::restoreSelected);
    selLayout->addWidget(btnRestore);

    btnDelete = new QPushButton("Move to Trash", selectionControlBar);
    styleActionBtn(btnDelete, "#dc3545", "#bb2d3b"); // Red
    connect(btnDelete, &QPushButton::clicked, this, &GalleryWindow::deleteSelected);
    selLayout->addWidget(btnDelete);

    btnDeletePermanently = new QPushButton("Delete Permanently", selectionControlBar);
    styleActionBtn(btnDeletePermanently, "#dc3545", "#bb2d3b"); // Solid Red
    connect(btnDeletePermanently, &QPushButton::clicked, this, &GalleryWindow::deletePermanentlySelected);
    selLayout->addWidget(btnDeletePermanently);

    selectionControlBar->setVisible(false);
    gridScreenLayout->addWidget(selectionControlBar);

    // Scroll Area
    scrollArea = new QScrollArea(gridScreen);
    scrollArea->setWidgetResizable(true);

    scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background-color: #ffffff;");
    scrollVLayout = new QVBoxLayout(scrollContent);
    scrollVLayout->setContentsMargins(24, 24, 24, 24);
    scrollVLayout->setSpacing(24);

    scrollArea->setWidget(scrollContent);
    gridScreenLayout->addWidget(scrollArea, 1);
    stackedWidget->addWidget(gridScreen);

    // ==========================================
    // SCREEN 2: DETAIL VIEW SCREEN
    // ==========================================
    detailScreen = new QWidget(stackedWidget);
    QHBoxLayout *detailLayout = new QHBoxLayout(detailScreen);
    detailLayout->setContentsMargins(0, 0, 0, 0);
    detailLayout->setSpacing(0);

    // Image viewer display area
    QWidget *imageContainer = new QWidget(detailScreen);
    imageContainer->setStyleSheet("background-color: #ffffff;");
    QVBoxLayout *imgContainerLayout = new QVBoxLayout(imageContainer);
    imgContainerLayout->setContentsMargins(20, 20, 20, 20);

    QFrame *imgFrame = new QFrame(imageContainer);
    imgFrame->setStyleSheet("QFrame { background-color: #f1f3f5; border: 1px solid #e9ecef; border-radius: 12px; }");
    QVBoxLayout *frameLayout = new QVBoxLayout(imgFrame);
    frameLayout->setContentsMargins(8, 8, 8, 8);

    lblDetailImage = new QLabel(imgFrame);
    lblDetailImage->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(lblDetailImage);

    imgContainerLayout->addWidget(imgFrame);
    detailLayout->addWidget(imageContainer, 1);

    // Sidebar panel
    QWidget *sidebar = new QWidget(detailScreen);
    sidebar->setFixedWidth(320);
    sidebar->setStyleSheet(
        "QWidget {"
        "  background-color: #f8f9fa;"
        "  border-left: 1px solid #dee2e6;"
        "}"
        "QLabel {"
        "  font-size: 13px;"
        "  color: #495057;"
        "}"
    );
    QVBoxLayout *metaSideLayout = new QVBoxLayout(sidebar);
    metaSideLayout->setContentsMargins(24, 30, 24, 30);
    metaSideLayout->setSpacing(16);

    QLabel *lblMetaTitle = new QLabel("IMAGE PROPERTIES", sidebar);
    lblMetaTitle->setStyleSheet("color: #212529; font-size: 14px; font-weight: 800; letter-spacing: 0.5px; margin-bottom: 10px;");
    metaSideLayout->addWidget(lblMetaTitle);

    QFrame *divider = new QFrame(sidebar);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #e9ecef; max-height: 1px; border: none;");
    metaSideLayout->addWidget(divider);

    auto addMetaLabel = [metaSideLayout, sidebar](const QString &title, QLabel *&valLabel) {
        QLabel *titleLbl = new QLabel(title, sidebar);
        titleLbl->setStyleSheet("font-weight: bold; color: #868e96; font-size: 11px; margin-top: 6px;");
        metaSideLayout->addWidget(titleLbl);
        valLabel = new QLabel("-", sidebar);
        valLabel->setStyleSheet("color: #212529; font-size: 13px; font-weight: 500; margin-bottom: 6px; word-wrap: break-word;");
        valLabel->setWordWrap(true);
        metaSideLayout->addWidget(valLabel);
    };

    addMetaLabel("FILE NAME", lblFilename);
    addMetaLabel("FULL PATH", lblPath);
    addMetaLabel("CAPTURED AT", lblCreated);
    addMetaLabel("RESOLUTION", lblDimensions);
    addMetaLabel("APPLIED FILTER", lblFilter);
    addMetaLabel("COMPRESSION MODE", lblCompression);
    addMetaLabel("FILE SIZE", lblFileSize);

    metaSideLayout->addStretch();

    btnBack = new QPushButton("← Back to Gallery", sidebar);
    btnBack->setStyleSheet(
        "QPushButton {"
        "  color: #ffffff;"
        "  background-color: #0d6efd;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 12px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #0b5ed7;"
        "}"
    );
    connect(btnBack, &QPushButton::clicked, this, &GalleryWindow::showGridView);
    metaSideLayout->addWidget(btnBack);

    detailLayout->addWidget(sidebar);
    stackedWidget->addWidget(detailScreen);
}

// --- Sidebar Tab triggers ---
void GalleryWindow::selectPhotosTab() {
    btnTabPhotos->setChecked(true);
    btnTabVideos->setChecked(false);
    btnTabTrash->setChecked(false);
    
    currentTab = TAB_PHOTOS;
    lblTitle->setText("PHOTOS");
    
    selectedFiles.clear();
    updateSelectionUI();
    refreshGallery();
}

void GalleryWindow::selectVideosTab() {
    btnTabPhotos->setChecked(false);
    btnTabVideos->setChecked(true);
    btnTabTrash->setChecked(false);
    
    currentTab = TAB_VIDEOS;
    lblTitle->setText("VIDEOS");
    
    selectedFiles.clear();
    updateSelectionUI();
    refreshGallery();
}

void GalleryWindow::selectTrashTab() {
    btnTabPhotos->setChecked(false);
    btnTabVideos->setChecked(false);
    btnTabTrash->setChecked(true);
    
    currentTab = TAB_TRASH;
    lblTitle->setText("TRASH BIN");
    
    selectedFiles.clear();
    updateSelectionUI();
    refreshGallery();
}

void GalleryWindow::refreshGallery() {
    QString targetPath = "/home/zenx/Desktop/My Images";
    if (currentTab == TAB_TRASH) {
        targetPath += "/.trash";
    }
    
    QDir dir(targetPath);
    QFileInfoList files;
    
    if (currentTab != TAB_VIDEOS) {
        QStringList filters;
        filters << "*.pff";
        files = dir.entryInfoList(filters, QDir::Files, QDir::Time); // Newest first
    }
    
    currentFiles = files;
    populateGrid(files);
}

void GalleryWindow::populateGrid(const QFileInfoList &files) {
    // Clean old layout and lists
    QLayoutItem *child;
    while ((child = scrollVLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
    galleryItems.clear();

    if (files.isEmpty()) {
        QLabel *lblEmpty = new QLabel(scrollContent);
        lblEmpty->setAlignment(Qt::AlignCenter);
        lblEmpty->setStyleSheet("color: #868e96; font-size: 16px; font-weight: 500; line-height: 1.5; margin-top: 120px;");
        
        if (currentTab == TAB_PHOTOS) {
            lblEmpty->setText("Your gallery is empty.\nTake photos with your camera to see them here!");
        } else if (currentTab == TAB_VIDEOS) {
            lblEmpty->setText("No videos captured yet.");
        } else if (currentTab == TAB_TRASH) {
            lblEmpty->setText("Trash bin is empty.");
        }
        scrollVLayout->addWidget(lblEmpty, 0, Qt::AlignCenter);
        return;
    }

    // Group files by Date (Day)
    QMap<QDate, QFileInfoList> groupedFiles;
    for (const QFileInfo &fileInfo : files) {
        QDate day = fileInfo.lastModified().date();
        groupedFiles[day].append(fileInfo);
    }

    // Sort dates in reverse (newest date section first)
    QList<QDate> dates = groupedFiles.keys();
    std::sort(dates.begin(), dates.end(), std::greater<QDate>());

    int columns = std::max(1, (width() - 250) / 240);

    for (const QDate &date : dates) {
        // Create section container
        QWidget *sectionWidget = new QWidget(scrollContent);
        QVBoxLayout *sectionLayout = new QVBoxLayout(sectionWidget);
        sectionLayout->setContentsMargins(0, 0, 0, 16);
        sectionLayout->setSpacing(12);

        // Date Header Label
        QString dateHeaderStr;
        QDate today = QDate::currentDate();
        if (date == today) {
            dateHeaderStr = "TODAY - " + date.toString("MMMM d, yyyy").toUpper();
        } else if (date == today.addDays(-1)) {
            dateHeaderStr = "YESTERDAY - " + date.toString("MMMM d, yyyy").toUpper();
        } else {
            dateHeaderStr = date.toString("dddd, MMMM d, yyyy").toUpper();
        }

        QLabel *lblHeader = new QLabel(dateHeaderStr, sectionWidget);
        lblHeader->setStyleSheet(
            "color: #495057;"
            "font-size: 13px;"
            "font-weight: 800;"
            "letter-spacing: 0.8px;"
            "padding-bottom: 4px;"
            "border-bottom: 1px solid #e9ecef;"
        );
        sectionLayout->addWidget(lblHeader);

        // Section Grid Widget
        QWidget *gridWidget = new QWidget(sectionWidget);
        QGridLayout *sectGridLayout = new QGridLayout(gridWidget);
        sectGridLayout->setContentsMargins(0, 8, 0, 0);
        sectGridLayout->setSpacing(16);

        int row = 0;
        int col = 0;

        for (const QFileInfo &fileInfo : groupedFiles[date]) {
            QString path = fileInfo.absoluteFilePath();
            QPixmap thumbPixmap;

            // Load from cache or extract
            if (thumbnailCache.contains(path)) {
                thumbPixmap = thumbnailCache[path];
            } else {
                pff::Image img;
                if (pff::load_pff(path.toStdString(), img)) {
                    QImage qimg(img.pixels.data(), img.header.width, img.header.height, QImage::Format_RGB888);
                    if (!qimg.isNull()) {
                        thumbPixmap = QPixmap::fromImage(qimg).scaled(200, 150, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                        if (thumbPixmap.width() > 200 || thumbPixmap.height() > 150) {
                            int x = (thumbPixmap.width() - 200) / 2;
                            int y = (thumbPixmap.height() - 150) / 2;
                            thumbPixmap = thumbPixmap.copy(x, y, 200, 150);
                        }
                        thumbnailCache[path] = thumbPixmap;
                    }
                }
            }

            if (thumbPixmap.isNull()) {
                QImage placeholder(200, 150, QImage::Format_RGB888);
                placeholder.fill(QColor(241, 243, 245));
                QPainter p(&placeholder);
                p.setPen(QColor(134, 142, 150));
                p.drawText(placeholder.rect(), Qt::AlignCenter, "CORRUPT PFF");
                thumbPixmap = QPixmap::fromImage(placeholder);
            }

            // Create custom grid item
            GalleryItem *item = new GalleryItem(path, thumbPixmap, fileInfo.fileName(), gridWidget);
            
            // Connect signals
            connect(item, &GalleryItem::itemPressed, this, &GalleryWindow::handleItemPressed);
            connect(item, &GalleryItem::itemDoubleClicked, this, &GalleryWindow::onImageDoubleClicked);
            
            // Restore selection highlight state if already in set
            if (selectedFiles.contains(path)) {
                item->setSelectedState(true);
            }

            sectGridLayout->addWidget(item, row, col);
            galleryItems.append(item);

            col++;
            if (col >= columns) {
                col = 0;
                row++;
            }
        }
        sectionLayout->addWidget(gridWidget);
        scrollVLayout->addWidget(sectionWidget);
    }
    
    // Add vertical spacer to push sections to the top
    scrollVLayout->addStretch(1);
}

void GalleryWindow::handleItemPressed(const QString &filepath, bool ctrlHeld) {
    if (ctrlHeld) {
        // Toggle selection
        if (selectedFiles.contains(filepath)) {
            selectedFiles.remove(filepath);
        } else {
            selectedFiles.insert(filepath);
        }
    } else {
        // Single selection
        selectedFiles.clear();
        selectedFiles.insert(filepath);
    }

    // Refresh highlighting on all instantiated items
    for (GalleryItem *item : galleryItems) {
        item->setSelectedState(selectedFiles.contains(item->filepath()));
    }

    updateSelectionUI();
}

void GalleryWindow::updateSelectionUI() {
    int count = selectedFiles.size();
    
    if (count == 0) {
        selectionControlBar->setVisible(false);
    } else {
        lblSelectionCount->setText(QString("%1 items selected").arg(count));
        selectionControlBar->setVisible(true);

        if (currentTab == TAB_PHOTOS) {
            btnDelete->setVisible(true);
            btnRestore->setVisible(false);
            btnDeletePermanently->setVisible(false);
        } else if (currentTab == TAB_TRASH) {
            btnDelete->setVisible(false);
            btnRestore->setVisible(true);
            btnDeletePermanently->setVisible(true);
        }
    }
}

void GalleryWindow::deleteSelected() {
    if (selectedFiles.isEmpty()) return;

    QString trashDirStr = "/home/zenx/Desktop/My Images/.trash";
    QDir().mkpath(trashDirStr);

    for (const QString &filepath : selectedFiles) {
        QFileInfo fi(filepath);
        QString destPath = trashDirStr + "/" + fi.fileName();
        
        // Move file to .trash directory
        QFile::rename(filepath, destPath);
        
        // Clear from cache
        thumbnailCache.remove(filepath);
    }

    selectedFiles.clear();
    updateSelectionUI();
    refreshGallery();
}

void GalleryWindow::restoreSelected() {
    if (selectedFiles.isEmpty()) return;

    QString photosDirStr = "/home/zenx/Desktop/My Images";

    for (const QString &filepath : selectedFiles) {
        QFileInfo fi(filepath);
        QString destPath = photosDirStr + "/" + fi.fileName();
        
        // Move back to main directory
        QFile::rename(filepath, destPath);
        
        // Clear from cache
        thumbnailCache.remove(filepath);
    }

    selectedFiles.clear();
    updateSelectionUI();
    refreshGallery();
}

void GalleryWindow::deletePermanentlySelected() {
    if (selectedFiles.isEmpty()) return;

    for (const QString &filepath : selectedFiles) {
        // Delete permanently
        QFile::remove(filepath);
        thumbnailCache.remove(filepath);
    }

    selectedFiles.clear();
    updateSelectionUI();
    refreshGallery();
}

void GalleryWindow::onImageDoubleClicked(const QString &filepath) {
    pff::Image img;
    if (!pff::load_pff(filepath.toStdString(), img)) {
        return;
    }

    QImage qimg(img.pixels.data(), img.header.width, img.header.height, QImage::Format_RGB888);
    if (qimg.isNull()) return;

    QPixmap fullPix = QPixmap::fromImage(qimg);
    
    // Fit image in viewer container dynamically
    int maxW = lblDetailImage->parentWidget()->width();
    int maxH = lblDetailImage->parentWidget()->height();
    if (maxW <= 0) maxW = 750;
    if (maxH <= 0) maxH = 550;
    
    lblDetailImage->setPixmap(fullPix.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Fill metadata inspector
    QFileInfo fileInfo(filepath);
    lblFilename->setText(fileInfo.fileName());
    lblPath->setText(filepath);
    
    std::time_t raw_time = static_cast<std::time_t>(img.header.timestamp);
    std::tm* timeinfo = std::localtime(&raw_time);
    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    lblCreated->setText(QString::fromLocal8Bit(time_str));
    
    lblDimensions->setText(QString("%1 x %2 px").arg(img.header.width).arg(img.header.height));
    lblFilter->setText(getFilterName(img.header.filter_type));
    lblCompression->setText(img.header.compression == pff::COMPRESS_RLE ? "RLE Compressed" : "Uncompressed Raw");
    lblFileSize->setText(formatSize(fileInfo.size()));

    // Slide view
    stackedWidget->setCurrentIndex(1);
}

void GalleryWindow::showGridView() {
    stackedWidget->setCurrentIndex(0);
}

void GalleryWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (stackedWidget->currentIndex() == 0) {
        populateGrid(currentFiles);
    }
}

QString GalleryWindow::formatSize(qint64 bytes) {
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    double kb = bytes / 1024.0;
    if (kb < 1024) return QString("%1 KB").arg(QString::number(kb, 'f', 1));
    double mb = kb / 1024.0;
    return QString("%1 MB").arg(QString::number(mb, 'f', 1));
}

QString GalleryWindow::getFilterName(int filterId) {
    switch (filterId) {
        case 0: return "Normal (None)";
        case 1: return "Grayscale (Mono)";
        case 2: return "Sepia";
        case 3: return "Inverted Color";
        case 4: return "Warm Ambient";
        case 5: return "Cool Ambient";
        default: return "Custom Filter";
    }
}

#include "MainWindow.h"
#include "TextEditor.h"
#include "ImageViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QStorageInfo>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QApplication>
#include <QItemSelectionModel>
#include <QClipboard>
#include <iostream>

void populateSandboxIfEmpty(const QString& rootPath) {
    QDir root(rootPath);
    if (!root.exists()) {
        root.mkpath(".");
    }
    
    if (root.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
        std::cout << "Populating sandbox filesystem..." << std::endl;
        
        // Projects Directory
        root.mkpath("Projects/Antigravity-Engine/src");
        root.mkpath("Projects/Antigravity-Engine/include");
        root.mkpath("Projects/Antigravity-Engine/tests");
        
        // Write README
        QFile readme(rootPath + "/Projects/Antigravity-Engine/README.md");
        if (readme.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&readme);
            out << "# Antigravity Simulation Engine\n\n"
                << "This is a high-performance simulation engine written in C++23.\n\n"
                << "## Build Instructions\n"
                << "```bash\n"
                << "mkdir build && cd build\n"
                << "cmake ..\n"
                << "make -j$(nproc)\n"
                << "```\n\n"
                << "## Key Features\n"
                << "- Multi-threaded physics solver\n"
                << "- Real-time voxel octree rendering\n"
                << "- SIMD accelerated vector math operations\n"
                << "- Cross-platform compatibility (Linux, Windows, macOS)\n";
            readme.close();
        }

        // Write Source Files
        QFile mainFile(rootPath + "/Projects/Antigravity-Engine/src/main.cpp");
        if (mainFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&mainFile);
            out << "#include <iostream>\n"
                << "#include \"engine.h\"\n\n"
                << "int main() {\n"
                << "    std::cout << \"Initializing Antigravity Engine v2.0...\" << std::endl;\n"
                << "    Engine engine;\n"
                << "    engine.run();\n"
                << "    return 0;\n"
                << "}\n";
            mainFile.close();
        }

        QFile engineCpp(rootPath + "/Projects/Antigravity-Engine/src/engine.cpp");
        if (engineCpp.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&engineCpp);
            out << "#include \"engine.h\"\n"
                << "#include <iostream>\n\n"
                << "Engine::Engine() : running(false) {}\n\n"
                << "void Engine::run() {\n"
                << "    running = true;\n"
                << "    std::cout << \"Engine is running!\" << std::endl;\n"
                << "}\n";
            engineCpp.close();
        }

        QFile engineH(rootPath + "/Projects/Antigravity-Engine/include/engine.h");
        if (engineH.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&engineH);
            out << "#pragma once\n\n"
                << "class Engine {\n"
                << "public:\n"
                << "    Engine();\n"
                << "    void run();\n"
                << "private:\n"
                << "    bool running;\n"
                << "};\n";
            engineH.close();
        }

        // Cyber Dashboard Web Project
        root.mkpath("Projects/Cyber-Dashboard/assets");
        QFile indexHtml(rootPath + "/Projects/Cyber-Dashboard/index.html");
        if (indexHtml.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&indexHtml);
            out << "<!DOCTYPE html>\n"
                << "<html lang=\"en\">\n"
                << "<head>\n"
                << "    <meta charset=\"UTF-8\">\n"
                << "    <title>Cyber Dashboard Mockup</title>\n"
                << "    <link rel=\"stylesheet\" href=\"style.css\">\n"
                << "</head>\n"
                << "<body>\n"
                << "    <div class=\"container\">\n"
                << "        <h1>Cyber System Status</h1>\n"
                << "        <div class=\"indicator active\">Online</div>\n"
                << "        <div id=\"app\">Loading telemetry...</div>\n"
                << "    </div>\n"
                << "    <script src=\"app.js\"></script>\n"
                << "</body>\n"
                << "</html>\n";
            indexHtml.close();
        }

        // style.css
        QFile styleCss(rootPath + "/Projects/Cyber-Dashboard/style.css");
        if (styleCss.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&styleCss);
            out << "body {\n"
                << "    background: #0d0f12;\n"
                << "    color: #00ffcc;\n"
                << "    font-family: 'Courier New', monospace;\n"
                << "    display: flex;\n"
                << "    justify-content: center;\n"
                << "    align-items: center;\n"
                << "    height: 100vh;\n"
                << "    margin: 0;\n"
                << "}\n"
                << ".container {\n"
                << "    border: 1px solid #00ffcc;\n"
                << "    padding: 30px;\n"
                << "    border-radius: 8px;\n"
                << "    background: rgba(0, 255, 204, 0.05);\n"
                << "    box-shadow: 0 0 20px rgba(0, 255, 204, 0.2);\n"
                << "}\n";
            styleCss.close();
        }

        // app.js
        QFile appJs(rootPath + "/Projects/Cyber-Dashboard/app.js");
        if (appJs.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&appJs);
            out << "console.log('Dashboard Telemetry Active.');\n";
            appJs.close();
        }

        // Documents
        root.mkpath("Documents/Reports");
        root.mkpath("Documents/Personal");
        root.mkpath("Documents/References");

        QFile todoFile(rootPath + "/Documents/todo.txt");
        if (todoFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&todoFile);
            out << "Antigravity Qt File Manager Roadmap:\n"
                << "---------------------------------\n"
                << "1. Implement fully functional native C++ Qt5 interface [DONE]\n"
                << "2. Add Quick Access sidebar with sandbox navigation [DONE]\n"
                << "3. Connect native filesystem sorting model [DONE]\n"
                << "4. Create custom file viewer for images & code files [DONE]\n"
                << "5. Integrate real-time storage bar metrics [DONE]\n"
                << "6. Establish file create, rename, delete actions [DONE]\n";
            todoFile.close();
        }

        QFile journal(rootPath + "/Documents/Personal/Journal.txt");
        if (journal.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&journal);
            out << "2026-06-20: Transitioned File Manager app into a native C++ Qt desktop app.\n"
                << "Now it feels extremely snappy and runs natively as pure C++ without dependencies.\n";
            journal.close();
        }

        // Downloads
        root.mkpath("Downloads");
        QFile mockZip(rootPath + "/Downloads/boost_1_85_0.zip");
        if (mockZip.open(QIODevice::WriteOnly)) {
            mockZip.write("MOCK ZIP ARCHIVE DATA");
            mockZip.close();
        }

        // Media Wallpapers
        root.mkpath("Media/Wallpapers");
        root.mkpath("Media/Music");

        QFile wallpaper(rootPath + "/Media/Wallpapers/cosmic_aurora.svg");
        if (wallpaper.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&wallpaper);
            out << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100' width='300' height='300'>\n"
                << "  <rect width='100' height='100' fill='#0B0914'/>\n"
                << "  <circle cx='50' cy='50' r='30' fill='#00ffcc' opacity='0.7'/>\n"
                << "</svg>\n";
            wallpaper.close();
        }
    }
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_sandboxRoot = "/home/zenx";
    m_currentPath = m_sandboxRoot;
    m_clipboardPath = "";
    m_clipboardIsCut = false;
    
    setupUI();
    setupConnections();
    
    changeDirectory(m_currentPath, false);
    updateStorage();
}

void MainWindow::setupUI() {
    setWindowTitle("Antigravity File Explorer [C++]");
    resize(950, 650);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(6);

    // 1. Navigation and Address Bar Layout
    QHBoxLayout* navLayout = new QHBoxLayout();
    navLayout->setSpacing(6);

    m_btnBack = new QPushButton("◀", this);
    m_btnBack->setObjectName("btnBack");
    m_btnBack->setToolTip("Go Back");
    m_btnBack->setEnabled(false);

    m_btnForward = new QPushButton("▶", this);
    m_btnForward->setObjectName("btnForward");
    m_btnForward->setToolTip("Go Forward");
    m_btnForward->setEnabled(false);

    m_btnUp = new QPushButton("▲", this);
    m_btnUp->setObjectName("btnUp");
    m_btnUp->setToolTip("Go Up");
    m_btnUp->setEnabled(false);

    m_btnRefresh = new QPushButton("↻", this);
    m_btnRefresh->setObjectName("btnRefresh");
    m_btnRefresh->setToolTip("Refresh");

    QFrame* sepNav = new QFrame();
    sepNav->setFrameShape(QFrame::VLine);
    sepNav->setStyleSheet("color: #d1d5db; max-width: 1px; margin: 2px 4px;");

    m_addressLabel = new QLabel(this);
    m_addressLabel->setObjectName("addressLabel");
    m_addressLabel->setText("Home >");
    m_addressLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_addressLabel->setMinimumWidth(200);

    m_searchBar = new QLineEdit(this);
    m_searchBar->setObjectName("searchBar");
    m_searchBar->setPlaceholderText("Search");
    m_searchBar->setFixedWidth(180);

    navLayout->addWidget(m_btnBack);
    navLayout->addWidget(m_btnForward);
    navLayout->addWidget(m_btnUp);
    navLayout->addWidget(m_btnRefresh);
    navLayout->addWidget(sepNav);
    navLayout->addWidget(m_addressLabel, 1);
    navLayout->addWidget(m_searchBar);
    mainLayout->addLayout(navLayout);

    // 2. Action commands layout
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(6);

    // "New" Button
    m_btnNew = new QPushButton("⊕ New ▾", this);
    m_btnNew->setObjectName("btnNew");
    QMenu* newMenu = new QMenu(this);
    newMenu->addAction("Folder");
    newMenu->addAction("File");
    m_btnNew->setMenu(newMenu);

    QFrame* sepAction1 = new QFrame();
    sepAction1->setFrameShape(QFrame::VLine);
    sepAction1->setStyleSheet("color: #d1d5db; max-width: 1px; margin: 2px 4px;");

    // Action buttons
    m_btnCut = new QPushButton("Cut", this);
    m_btnCut->setObjectName("btnCut");
    m_btnCut->setEnabled(false);
    m_btnCopy = new QPushButton("Copy", this);
    m_btnCopy->setObjectName("btnCopy");
    m_btnCopy->setEnabled(false);
    m_btnPaste = new QPushButton("Paste", this);
    m_btnPaste->setObjectName("btnPaste");
    m_btnPaste->setEnabled(false);
    m_btnRename = new QPushButton("Rename", this);
    m_btnRename->setObjectName("btnRename");
    m_btnRename->setEnabled(false);
    m_btnShare = new QPushButton("Share", this);
    m_btnShare->setObjectName("btnShare");
    m_btnShare->setEnabled(false);
    m_btnDelete = new QPushButton("Delete", this);
    m_btnDelete->setObjectName("btnDelete");
    m_btnDelete->setEnabled(false);

    QFrame* sepAction2 = new QFrame();
    sepAction2->setFrameShape(QFrame::VLine);
    sepAction2->setStyleSheet("color: #d1d5db; max-width: 1px; margin: 2px 4px;");

    // View Button
    m_btnView = new QPushButton("☷ View ▾", this);
    m_btnView->setObjectName("btnView");
    QMenu* viewMenu = new QMenu(this);
    viewMenu->addAction("List");
    viewMenu->addAction("Tiles");
    m_btnView->setMenu(viewMenu);

    QFrame* sepAction3 = new QFrame();
    sepAction3->setFrameShape(QFrame::VLine);
    sepAction3->setStyleSheet("color: #d1d5db; max-width: 1px; margin: 2px 4px;");

    // Ellipsis button (3 dots)
    m_btnMore = new QPushButton("⋮", this);
    m_btnMore->setObjectName("btnMore");
    QMenu* moreMenu = new QMenu(this);
    m_actPin = moreMenu->addAction("Pin to Quick access");
    m_actPin->setEnabled(false);
    moreMenu->addSeparator();
    m_actSelectAll = moreMenu->addAction("Select all");
    m_actSelectNone = moreMenu->addAction("Select none");
    m_btnMore->setMenu(moreMenu);

    actionLayout->addWidget(m_btnNew);
    actionLayout->addWidget(sepAction1);
    actionLayout->addWidget(m_btnCut);
    actionLayout->addWidget(m_btnCopy);
    actionLayout->addWidget(m_btnPaste);
    actionLayout->addWidget(m_btnRename);
    actionLayout->addWidget(m_btnShare);
    actionLayout->addWidget(m_btnDelete);
    actionLayout->addWidget(sepAction2);
    actionLayout->addWidget(m_btnView);
    actionLayout->addWidget(sepAction3);
    actionLayout->addWidget(m_btnMore);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);

    // Separator line under the header controls
    QFrame* headerLine = new QFrame();
    headerLine->setFrameShape(QFrame::HLine);
    headerLine->setStyleSheet("background-color: #d1d5db; max-height: 1px; margin: 4px 0;");
    mainLayout->addWidget(headerLine);

    // Splitter (Sidebar + Main panel)
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // Sidebar
    QWidget* sidebarWidget = new QWidget(this);
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);

    m_sidebar = new QListWidget(this);
    m_sidebar->addItem("Home");

    // Separator under Home
    QListWidgetItem* sep1 = new QListWidgetItem(m_sidebar);
    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("background-color: #e5e7eb; max-height: 1px; margin: 4px 0;");
    m_sidebar->setItemWidget(sep1, line1);
    sep1->setFlags(Qt::NoItemFlags);

    m_sidebar->addItem("Desktop");
    m_sidebar->addItem("Downloads");
    m_sidebar->addItem("Documents");
    m_sidebar->addItem("Pictures");

    // Separator under bookmarks
    QListWidgetItem* sep2 = new QListWidgetItem(m_sidebar);
    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("background-color: #e5e7eb; max-height: 1px; margin: 4px 0;");
    m_sidebar->setItemWidget(sep2, line2);
    sep2->setFlags(Qt::NoItemFlags);

    m_sidebar->addItem("My Pc");

    // Quick Access Section (the second bubble)
    QLabel* quickLabel = new QLabel("Quick Access", this);
    quickLabel->setStyleSheet("font-weight: bold; font-size: 11px; margin-top: 12px; margin-bottom: 4px;");

    m_quickAccess = new QListWidget(this);
    m_quickAccess->setObjectName("quickAccess");

    m_storageLabel = nullptr;

    sidebarLayout->addWidget(m_sidebar, 4);
    sidebarLayout->addWidget(quickLabel);
    sidebarLayout->addWidget(m_quickAccess, 1);
    sidebarWidget->setLayout(sidebarLayout);

    // File System Model & Search Filter
    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setRootPath("/");
    m_fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_fileModel);
    m_proxyModel->setFilterKeyColumn(0);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // List/Grid Stack
    m_viewStack = new QStackedWidget(this);

    // View 1: List View (QTreeView)
    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_proxyModel);
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);
    m_treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
    m_treeView->setColumnWidth(0, 250);
    m_treeView->setColumnWidth(1, 100);
    m_treeView->setColumnWidth(2, 100);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_viewStack->addWidget(m_treeView);

    // View 2: Grid View (QListView in IconMode)
    m_listView = new QListView(this);
    m_listView->setModel(m_proxyModel);
    m_listView->setViewMode(QListView::IconMode);
    m_listView->setIconSize(QSize(48, 48));
    m_listView->setGridSize(QSize(90, 90));
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->setWordWrap(true);
    m_listView->setSpacing(8);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_viewStack->addWidget(m_listView);

    splitter->addWidget(sidebarWidget);
    splitter->addWidget(m_viewStack);
    splitter->setSizes(QList<int>() << 190 << 760);
    mainLayout->addWidget(splitter);

    // Status Bar
    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);
    m_statusBar->showMessage("File Manager ready.");

    // Load stylesheet from QSS file if exists, else apply default stylesheet
    QFile styleFile(QCoreApplication::applicationDirPath() + "/resources/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&styleFile);
        setStyleSheet(ts.readAll());
        styleFile.close();
    } else {
        // Fallback QSS
        setStyleSheet(
            "QMainWindow { background-color: #0b0914; }"
            "QSplitter::handle { background-color: #331f4d; }"
            "QListWidget { background-color: transparent; color: #e0e0e0; border: none; padding: 5px; }"
            "QListWidget::item { padding: 8px; border-radius: 4px; }"
            "QListWidget::item:hover { background-color: #221b3d; color: #00ffcc; }"
            "QListWidget::item:selected { background-color: #3b1d70; color: #00ffcc; border: 1px solid #00ffcc; }"
            "QTreeView { background-color: #131124; color: #e0e0e0; border: 1px solid #331f4d; border-radius: 8px; }"
            "QListView { background-color: #131124; color: #e0e0e0; border: 1px solid #331f4d; border-radius: 8px; }"
            "QLineEdit { background-color: #131124; color: #00ffcc; border: 1px solid #331f4d; border-radius: 6px; padding: 6px; }"
            "QPushButton { background-color: #1a1633; color: #e0e0e0; border: 1px solid #331f4d; border-radius: 6px; padding: 6px 12px; font-weight: bold; }"
            "QPushButton:hover { background-color: #2c1a4d; border-color: #00ffcc; color: #00ffcc; }"
            "QLabel { color: #e0e0e0; }"
        );
    }
}

void MainWindow::setupConnections() {
    connect(m_sidebar, &QListWidget::itemClicked, this, &MainWindow::onBookmarkClicked);
    connect(m_quickAccess, &QListWidget::itemClicked, this, &MainWindow::onQuickAccessClicked);
    
    connect(m_treeView, &QTreeView::doubleClicked, this, &MainWindow::onItemDoubleClicked);
    connect(m_listView, &QListView::doubleClicked, this, &MainWindow::onItemDoubleClicked);
    
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(m_listView, &QListView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    
    // Top bar connections
    connect(m_btnBack, &QPushButton::clicked, this, &MainWindow::onNavigateBack);
    connect(m_btnForward, &QPushButton::clicked, this, &MainWindow::onNavigateForward);
    connect(m_btnUp, &QPushButton::clicked, this, &MainWindow::onNavigateUp);
    connect(m_btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefresh);
    connect(m_searchBar, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // Selection connections
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);

    // Action connections
    connect(m_btnCut, &QPushButton::clicked, this, &MainWindow::onCut);
    connect(m_btnCopy, &QPushButton::clicked, this, &MainWindow::onCopy);
    connect(m_btnPaste, &QPushButton::clicked, this, &MainWindow::onPaste);
    connect(m_btnRename, &QPushButton::clicked, this, &MainWindow::onRename);
    connect(m_btnShare, &QPushButton::clicked, this, &MainWindow::onShare);
    connect(m_btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
    
    // View connections
    QList<QAction*> viewActions = m_btnView->menu()->actions();
    if (viewActions.size() >= 2) {
        connect(viewActions[0], &QAction::triggered, this, &MainWindow::onViewList);
        connect(viewActions[1], &QAction::triggered, this, &MainWindow::onViewGrid);
    }
    
    // New menu connections
    QList<QAction*> newActions = m_btnNew->menu()->actions();
    if (newActions.size() >= 2) {
        connect(newActions[0], &QAction::triggered, this, &MainWindow::onNewFolder);
        connect(newActions[1], &QAction::triggered, this, &MainWindow::onNewFile);
    }

    // More menu connections
    connect(m_actPin, &QAction::triggered, this, &MainWindow::onPinToQuickAccess);
    connect(m_actSelectAll, &QAction::triggered, this, &MainWindow::onSelectAll);
    connect(m_actSelectNone, &QAction::triggered, this, &MainWindow::onSelectNone);
}

void MainWindow::onBookmarkClicked(QListWidgetItem* item) {
    if (item->flags() == Qt::NoItemFlags) return;
    
    QString name = item->text();
    QString target = "/home/zenx";
    if (name == "Desktop") target = "/home/zenx/Desktop";
    else if (name == "Downloads") target = "/home/zenx/Downloads";
    else if (name == "Documents") target = "/home/zenx/Documents";
    else if (name == "Pictures") target = "/home/zenx/Pictures";
    else if (name == "My Pc") target = "/";

    changeDirectory(target);
    m_statusBar->showMessage("Loaded folder: " + name);
}

void MainWindow::onQuickAccessClicked(QListWidgetItem* item) {
    if (!item) return;
    QString target = item->data(Qt::UserRole).toString();
    if (target.isEmpty()) return;
    
    changeDirectory(target);
    m_statusBar->showMessage("Loaded Quick Access: " + item->text());
}

void MainWindow::onItemDoubleClicked(const QModelIndex& proxyIndex) {
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    if (m_fileModel->isDir(sourceIndex)) {
        changeDirectory(m_fileModel->filePath(sourceIndex));
        m_statusBar->showMessage("Entered: " + m_fileModel->fileName(sourceIndex));
    } else {
        openFile(m_fileModel->filePath(sourceIndex));
    }
}

void MainWindow::openFile(const QString& filePath) {
    QFileInfo info(filePath);
    QString ext = info.suffix().toLower();

    if (ext == "txt" || ext == "cpp" || ext == "h" || ext == "md" || ext == "json" || ext == "html" || ext == "css" || ext == "js") {
        TextEditor dlg(filePath, this);
        dlg.exec();
        updateStorage();
    } else if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif" || ext == "svg") {
        ImageViewer dlg(filePath, this);
        dlg.exec();
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        m_statusBar->showMessage("Opened: " + info.fileName());
    }
}

void MainWindow::updateStorage() {
    if (!m_storageLabel) return;
    QStorageInfo storage(m_sandboxRoot);
    if (storage.isValid()) {
        qint64 total = storage.bytesTotal();
        qint64 free = storage.bytesFree();
        qint64 used = total - free;
        
        double usedGb = used / (1024.0 * 1024.0 * 1024.0);
        double totalGb = total / (1024.0 * 1024.0 * 1024.0);
        
        m_storageLabel->setText(QString("%1 GB / %2 GB used").arg(usedGb, 0, 'f', 1).arg(totalGb, 0, 'f', 1));
    } else {
        m_storageLabel->setText("Storage metrics offline");
    }
}

void MainWindow::populateSandbox(const QString& rootPath) {
    populateSandboxIfEmpty(rootPath);
}

// Path Helpers
void MainWindow::changeDirectory(const QString& path, bool recordHistory) {
    QDir dir(path);
    if (!dir.exists()) return;

    if (recordHistory && m_currentPath != path) {
        m_historyBack.append(m_currentPath);
        m_historyForward.clear();
    }
    
    m_currentPath = path;
    
    QModelIndex sourceIndex = m_fileModel->index(m_currentPath);
    if (sourceIndex.isValid()) {
        QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
        m_treeView->setRootIndex(proxyIndex);
        m_listView->setRootIndex(proxyIndex);
    }
    
    // Update nav button states
    m_btnBack->setEnabled(!m_historyBack.isEmpty());
    m_btnForward->setEnabled(!m_historyForward.isEmpty());
    
    QModelIndex sourceParent = sourceIndex.parent();
    m_btnUp->setEnabled(sourceParent.isValid());
    
    updateAddressBar();

    // Update search placeholder dynamically
    QFileInfo info(m_currentPath);
    QString folderName = info.fileName();
    if (folderName.isEmpty()) folderName = "Root";
    m_searchBar->setPlaceholderText("Search " + folderName);
    m_searchBar->clear();
    m_proxyModel->setFilterWildcard("");
    
    onSelectionChanged();
}

void MainWindow::updateAddressBar() {
    QString relativePath = m_currentPath;
    if (relativePath.startsWith(m_sandboxRoot)) {
        relativePath = relativePath.mid(m_sandboxRoot.length());
        if (relativePath.startsWith("/")) relativePath = relativePath.mid(1);
        relativePath = "Home > " + relativePath;
    } else {
        relativePath = "Root > " + relativePath;
    }
    
    relativePath.replace("/", " > ");
    if (!relativePath.endsWith(" > ") && relativePath != "Home" && relativePath != "Root") {
        relativePath += " >";
    }
    
    m_addressLabel->setText(relativePath);
}

bool MainWindow::copyRecursively(const QString& srcFilePath, const QString& tgtFilePath) {
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.mkpath(".");
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath = srcFilePath + "/" + fileName;
            const QString newTgtFilePath = tgtFilePath + "/" + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath)) {
                return false;
            }
        }
    } else {
        if (QFile::exists(tgtFilePath)) {
            QFile::remove(tgtFilePath);
        }
        if (!QFile::copy(srcFilePath, tgtFilePath)) {
            return false;
        }
    }
    return true;
}

// Top Bar Slots
void MainWindow::onNavigateBack() {
    if (m_historyBack.isEmpty()) return;
    QString prev = m_historyBack.takeLast();
    m_historyForward.append(m_currentPath);
    changeDirectory(prev, false);
    m_statusBar->showMessage("Navigated back.");
}

void MainWindow::onNavigateForward() {
    if (m_historyForward.isEmpty()) return;
    QString next = m_historyForward.takeLast();
    m_historyBack.append(m_currentPath);
    changeDirectory(next, false);
    m_statusBar->showMessage("Navigated forward.");
}

void MainWindow::onNavigateUp() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->rootIndex() : m_listView->rootIndex();
    QModelIndex sourceCurrent = m_proxyModel->mapToSource(currentProxy);
    QModelIndex sourceParent = sourceCurrent.parent();
    
    if (sourceParent.isValid()) {
        QString parentPath = m_fileModel->filePath(sourceParent);
        changeDirectory(parentPath);
        m_statusBar->showMessage("Navigated up to parent directory.");
    }
}

void MainWindow::onRefresh() {
    m_fileModel->setRootPath("");
    m_fileModel->setRootPath("/");
    changeDirectory(m_currentPath, false);
    m_statusBar->showMessage("Refreshed.");
}

void MainWindow::onSearchTextChanged(const QString& text) {
    if (text.isEmpty()) {
        m_proxyModel->setFilterWildcard("");
    } else {
        m_proxyModel->setFilterWildcard("*" + text + "*");
    }
}

void MainWindow::onSelectionChanged() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->currentIndex() : m_listView->currentIndex();
    bool hasSelection = currentProxy.isValid();
    
    m_btnCut->setEnabled(hasSelection);
    m_btnCopy->setEnabled(hasSelection);
    m_btnRename->setEnabled(hasSelection);
    m_btnShare->setEnabled(hasSelection);
    m_btnDelete->setEnabled(hasSelection);
    
    m_btnPaste->setEnabled(!m_clipboardPath.isEmpty());
    
    bool isFolder = false;
    if (hasSelection) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxy);
        isFolder = m_fileModel->isDir(sourceIndex);
    }
    m_actPin->setEnabled(isFolder);
}

void MainWindow::onNewFolder() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Folder Name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QDir dir(m_currentPath);
        if (dir.mkdir(name)) {
            m_statusBar->showMessage("Folder created: " + name);
            updateStorage();
        } else {
            QMessageBox::warning(this, "Create Error", "Could not create folder. Name might be occupied.");
        }
    }
}

void MainWindow::onNewFile() {
    bool ok;
    QString name = QInputDialog::getText(this, "New File", "File Name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QFile file(m_currentPath + "/" + name);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.close();
            m_statusBar->showMessage("File created: " + name);
            updateStorage();
        } else {
            QMessageBox::warning(this, "Create Error", "Could not create empty file.");
        }
    }
}

void MainWindow::onCut() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->currentIndex() : m_listView->currentIndex();
    if (currentProxy.isValid()) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxy);
        m_clipboardPath = m_fileModel->filePath(sourceIndex);
        m_clipboardIsCut = true;
        m_btnPaste->setEnabled(true);
        m_statusBar->showMessage("Cut: " + m_fileModel->fileName(sourceIndex));
    }
}

void MainWindow::onCopy() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->currentIndex() : m_listView->currentIndex();
    if (currentProxy.isValid()) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxy);
        m_clipboardPath = m_fileModel->filePath(sourceIndex);
        m_clipboardIsCut = false;
        m_btnPaste->setEnabled(true);
        m_statusBar->showMessage("Copied: " + m_fileModel->fileName(sourceIndex));
    }
}

void MainWindow::onPaste() {
    if (m_clipboardPath.isEmpty()) return;
    
    QFileInfo srcInfo(m_clipboardPath);
    QString destPath = m_currentPath + "/" + srcInfo.fileName();
    
    if (m_clipboardIsCut) {
        bool success = false;
        if (srcInfo.isDir()) {
            success = QDir().rename(m_clipboardPath, destPath);
        } else {
            success = QFile::rename(m_clipboardPath, destPath);
        }
        
        if (success) {
            m_statusBar->showMessage("Moved item to current folder.");
            m_clipboardPath.clear();
            m_btnPaste->setEnabled(false);
        } else {
            QMessageBox::warning(this, "Paste Error", "Failed to move the item.");
        }
    } else {
        bool success = false;
        if (srcInfo.isDir()) {
            success = copyRecursively(m_clipboardPath, destPath);
        } else {
            success = QFile::copy(m_clipboardPath, destPath);
        }
        
        if (success) {
            m_statusBar->showMessage("Copied item to current folder.");
        } else {
            QMessageBox::warning(this, "Paste Error", "Failed to copy the item. Destination file might already exist.");
        }
    }
    
    updateStorage();
}

void MainWindow::onRename() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->currentIndex() : m_listView->currentIndex();
    if (!currentProxy.isValid()) {
        QMessageBox::warning(this, "Rename Action", "Please select a file or directory first.");
        return;
    }

    QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxy);
    QString oldName = m_fileModel->fileName(sourceIndex);

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Item", "Enter New Name:", QLineEdit::Normal, oldName, &ok);
    if (ok && !newName.isEmpty() && newName != oldName) {
        QString parentPath = m_fileModel->filePath(sourceIndex.parent());
        QDir dir(parentPath);
        if (dir.rename(oldName, newName)) {
            m_statusBar->showMessage("Renamed: " + oldName + " -> " + newName);
            onSelectionChanged();
        } else {
            QMessageBox::warning(this, "Error", "Rename operation failed!");
        }
    }
}

void MainWindow::onShare() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->currentIndex() : m_listView->currentIndex();
    if (currentProxy.isValid()) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxy);
        QString filePath = m_fileModel->filePath(sourceIndex);
        
        QApplication::clipboard()->setText(filePath);
        
        QMessageBox::information(this, "Share Item", "Item path has been copied to system clipboard:\n" + filePath);
        m_statusBar->showMessage("Shared item path copied to clipboard.");
    }
}

void MainWindow::onDelete() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->currentIndex() : m_listView->currentIndex();
    if (!currentProxy.isValid()) {
        QMessageBox::warning(this, "Delete Action", "Please select a file or directory first.");
        return;
    }

    QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxy);
    QString itemName = m_fileModel->fileName(sourceIndex);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete Confirmation", "Are you sure you want to permanently delete: " + itemName + "?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        bool success = false;
        if (m_fileModel->isDir(sourceIndex)) {
            success = m_fileModel->rmdir(sourceIndex);
        } else {
            success = m_fileModel->remove(sourceIndex);
        }

        if (success) {
            m_statusBar->showMessage("Deleted: " + itemName);
            updateStorage();
            onSelectionChanged();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete selected item!");
        }
    }
}

void MainWindow::onViewList() {
    m_viewStack->setCurrentIndex(0);
    m_statusBar->showMessage("Switched to List View.");
}

void MainWindow::onViewGrid() {
    m_viewStack->setCurrentIndex(1);
    m_statusBar->showMessage("Switched to Tiles View.");
}

void MainWindow::onPinToQuickAccess() {
    QModelIndex currentProxy = m_viewStack->currentIndex() == 0 ? m_treeView->currentIndex() : m_listView->currentIndex();
    if (currentProxy.isValid()) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(currentProxy);
        if (m_fileModel->isDir(sourceIndex)) {
            QString path = m_fileModel->filePath(sourceIndex);
            QString name = m_fileModel->fileName(sourceIndex);
            
            bool exists = false;
            for (int i = 0; i < m_quickAccess->count(); ++i) {
                if (m_quickAccess->item(i)->data(Qt::UserRole).toString() == path) {
                    exists = true;
                    break;
                }
            }
            
            if (!exists) {
                QListWidgetItem* item = new QListWidgetItem(name, m_quickAccess);
                item->setData(Qt::UserRole, path);
                m_statusBar->showMessage("Pinned folder: " + name);
            } else {
                m_statusBar->showMessage("Folder already pinned: " + name);
            }
        }
    }
}

void MainWindow::onSelectAll() {
    if (m_viewStack->currentIndex() == 0) {
        m_treeView->selectAll();
    } else {
        m_listView->selectAll();
    }
}

void MainWindow::onSelectNone() {
    if (m_viewStack->currentIndex() == 0) {
        m_treeView->clearSelection();
    } else {
        m_listView->clearSelection();
    }
}

void MainWindow::showContextMenu(const QPoint& pos) {
    QWidget* senderWidget = qobject_cast<QWidget*>(sender());
    if (!senderWidget) return;

    QModelIndex proxyIndex;
    if (senderWidget == m_treeView) {
        proxyIndex = m_treeView->indexAt(pos);
    } else if (senderWidget == m_listView) {
        proxyIndex = m_listView->indexAt(pos);
    }

    if (!proxyIndex.isValid()) return;

    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    QString filePath = m_fileModel->filePath(sourceIndex);
    bool isFolder = m_fileModel->isDir(sourceIndex);

    QMenu menu(this);
    
    QAction* actCut = menu.addAction("Cut");
    QAction* actCopy = menu.addAction("Copy");
    QAction* actRename = menu.addAction("Rename");
    QAction* actDelete = menu.addAction("Delete");
    
    menu.addSeparator();
    
    QAction* actPin = menu.addAction("Pin to Quick access");
    actPin->setEnabled(isFolder);
    
    QMenu* compressMenu = menu.addMenu("Compress to..");
    QAction* actZip = compressMenu->addAction("ZIP file");
    
    QAction* actCopyPath = menu.addAction("Copy as path");

    // Select the item so actions act on it correctly
    if (senderWidget == m_treeView) {
        m_treeView->setCurrentIndex(proxyIndex);
    } else {
        m_listView->setCurrentIndex(proxyIndex);
    }
    onSelectionChanged();

    QAction* selectedAction = menu.exec(senderWidget->mapToGlobal(pos));
    if (!selectedAction) return;

    if (selectedAction == actCut) {
        onCut();
    } else if (selectedAction == actCopy) {
        onCopy();
    } else if (selectedAction == actRename) {
        onRename();
    } else if (selectedAction == actDelete) {
        onDelete();
    } else if (selectedAction == actPin) {
        onPinToQuickAccess();
    } else if (selectedAction == actZip) {
        // Zip compression helper
        QString zipPath = filePath + ".zip";
        QFile file(zipPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "MOCK ZIP ARCHIVE FOR: " << filePath << "\n";
            file.close();
            m_statusBar->showMessage("Compressed to: " + m_fileModel->fileName(sourceIndex) + ".zip");
            QMessageBox::information(this, "Compression Success", "Successfully compressed to:\n" + zipPath);
        } else {
            QMessageBox::warning(this, "Compression Error", "Could not create zip archive.");
        }
    } else if (selectedAction == actCopyPath) {
        QApplication::clipboard()->setText(filePath);
        m_statusBar->showMessage("Copied path to clipboard.");
    }
}

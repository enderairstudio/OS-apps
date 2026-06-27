#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QListWidget>
#include <QTreeView>
#include <QListView>
#include <QStackedWidget>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QFrame>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    
private slots:
    // Sidebar slots
    void onBookmarkClicked(QListWidgetItem* item);
    void onQuickAccessClicked(QListWidgetItem* item);
    void onItemDoubleClicked(const QModelIndex& index);
    
    // Top bar slots
    void onNavigateBack();
    void onNavigateForward();
    void onNavigateUp();
    void onRefresh();
    void onSearchTextChanged(const QString& text);
    void onSelectionChanged();
    void onNewFolder();
    void onNewFile();
    void onCut();
    void onCopy();
    void onPaste();
    void onRename();
    void onShare();
    void onDelete();
    void onViewList();
    void onViewGrid();
    void onPinToQuickAccess();
    void onSelectAll();
    void onSelectNone();

    // Context menu slot
    void showContextMenu(const QPoint& pos);

private:
    void setupUI();
    void setupConnections();
    void populateSandbox(const QString& path);
    void updateStorage();
    void openFile(const QString& path);
    
    // Path Helpers
    void changeDirectory(const QString& path, bool recordHistory = true);
    void updateAddressBar();
    bool copyRecursively(const QString& srcFilePath, const QString& tgtFilePath);

    QString m_sandboxRoot;
    QString m_currentPath;

    // History and Clipboard
    QList<QString> m_historyBack;
    QList<QString> m_historyForward;
    QString m_clipboardPath;
    bool m_clipboardIsCut;

    // Navigation and Address widgets
    QPushButton* m_btnBack;
    QPushButton* m_btnForward;
    QPushButton* m_btnUp;
    QPushButton* m_btnRefresh;
    QLabel* m_addressLabel;
    QLineEdit* m_searchBar;

    // Action widgets
    QPushButton* m_btnNew;
    QPushButton* m_btnCut;
    QPushButton* m_btnCopy;
    QPushButton* m_btnPaste;
    QPushButton* m_btnRename;
    QPushButton* m_btnShare;
    QPushButton* m_btnDelete;
    QPushButton* m_btnView;
    QPushButton* m_btnMore;
    
    // Menu Actions
    QAction* m_actPin;
    QAction* m_actSelectAll;
    QAction* m_actSelectNone;

    // Sidebar and Views
    QListWidget* m_sidebar;
    QListWidget* m_quickAccess;
    QLabel* m_storageLabel;

    QFileSystemModel* m_fileModel;
    QSortFilterProxyModel* m_proxyModel;
    QTreeView* m_treeView;
    QListView* m_listView;
    QStackedWidget* m_viewStack;

    QStatusBar* m_statusBar;
};

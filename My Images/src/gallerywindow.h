#ifndef GALLERYWINDOW_H
#define GALLERYWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QFileInfoList>
#include <QImage>
#include <QSet>
#include <QFrame>

class GalleryItem : public QFrame {
    Q_OBJECT
public:
    explicit GalleryItem(const QString &filepath, const QPixmap &thumb, const QString &filename, QWidget *parent = nullptr);
    void setSelectedState(bool selected);
    bool isSelected() const { return m_selected; }
    QString filepath() const { return m_filepath; }

signals:
    void itemPressed(const QString &filepath, bool ctrlHeld);
    void itemDoubleClicked(const QString &filepath);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QString m_filepath;
    bool m_selected;
    QLabel *lblThumb;
    QLabel *lblText;
    QFrame *overlayCheck;
};

class GalleryWindow : public QMainWindow {
    Q_OBJECT

public:
    enum SidebarTab {
        TAB_PHOTOS = 0,
        TAB_VIDEOS = 1,
        TAB_TRASH = 2
    };

    explicit GalleryWindow(QWidget *parent = nullptr);
    ~GalleryWindow() override;

private slots:
    void refreshGallery();
    void onImageDoubleClicked(const QString &filepath);
    void showGridView();
    
    // Sidebar slots
    void selectPhotosTab();
    void selectVideosTab();
    void selectTrashTab();

    // Selection & deletion slots
    void handleItemPressed(const QString &filepath, bool ctrlHeld);
    void deleteSelected();
    void restoreSelected();
    void deletePermanentlySelected();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUi();
    void updateSelectionUI();
    void populateGrid(const QFileInfoList &files);
    void updateSidebarIcons();
    void navigateViewer(int direction);
    
    // Helper to format file size
    QString formatSize(qint64 bytes);
    
    // Helper to get filter name
    QString getFilterName(int filterId);

    // Tab navigation variable
    SidebarTab currentTab;

    // UI elements
    QStackedWidget *stackedWidget;
    
    // Left Sidebar
    QWidget *leftSidebar;
    QPushButton *btnTabPhotos;
    QPushButton *btnTabVideos;
    QPushButton *btnTabTrash;
    
    // Screen 1: Grid View
    QWidget *gridScreen;
    QScrollArea *scrollArea;
    QWidget *scrollContent;
    QVBoxLayout *scrollVLayout; // Layout inside scroll content to hold date groups
    QPushButton *btnRefresh;
    QLabel *lblTitle;
    
    // Control buttons for multi-selection
    QWidget *selectionControlBar;
    QPushButton *btnDelete;
    QPushButton *btnRestore;
    QPushButton *btnDeletePermanently;
    QLabel *lblSelectionCount;
    
    // Screen 2: Detail View
    QWidget *detailScreen;
    QLabel *lblDetailImage;
    QLabel *lblFilename;
    QLabel *lblPath;
    QLabel *lblCreated;
    QLabel *lblDimensions;
    QLabel *lblFilter;
    QLabel *lblCompression;
    QLabel *lblFileSize;
    QPushButton *btnBack;

    // Filesystem monitoring
    QFileSystemWatcher *watcher;
    QFileInfoList currentFiles;
    
    // Selection state
    QSet<QString> selectedFiles;
    QList<GalleryItem*> galleryItems;
    
    // Thumbnail cache
    QMap<QString, QPixmap> thumbnailCache;
};

#endif // GALLERYWINDOW_H

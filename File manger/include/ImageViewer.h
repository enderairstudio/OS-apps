#pragma once
#include <QDialog>
#include <QLabel>
#include <QScrollArea>

class ImageViewer : public QDialog {
    Q_OBJECT
public:
    ImageViewer(const QString& filePath, QWidget* parent = nullptr);
};

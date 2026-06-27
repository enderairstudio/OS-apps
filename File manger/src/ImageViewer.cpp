#include "ImageViewer.h"
#include <QVBoxLayout>
#include <QPixmap>
#include <QFileInfo>

ImageViewer::ImageViewer(const QString& filePath, QWidget* parent)
    : QDialog(parent) {
    
    setWindowTitle("Preview - " + QFileInfo(filePath).fileName());
    resize(500, 500);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: #131124; border: 1px solid #331f4d; border-radius: 8px; }"
    );
    layout->addWidget(scrollArea);

    QLabel* label = new QLabel(scrollArea);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("background-color: transparent;");

    QPixmap pixmap(filePath);
    if (!pixmap.isNull()) {
        if (pixmap.width() > 700 || pixmap.height() > 700) {
            label->setPixmap(pixmap.scaled(700, 700, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            label->setPixmap(pixmap);
        }
    } else {
        label->setText("No image preview available.");
        label->setStyleSheet("color: #a0a0a0; font-size: 14px;");
    }

    scrollArea->setWidget(label);
}

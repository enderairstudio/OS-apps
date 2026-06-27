#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>

class Clip3DWidget : public QWidget {
    Q_OBJECT

public:
    explicit Clip3DWidget(QWidget *parent = nullptr);

signals:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    struct Point3D {
        float x;
        float y;
        float z;
    };

    QTimer timer_;
    QElapsedTimer elapsed_;
};

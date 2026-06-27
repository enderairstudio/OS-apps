#include "Clip3DWidget.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygonF>
#include <QtMath>

Clip3DWidget::Clip3DWidget(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    connect(&timer_, &QTimer::timeout, this, QOverload<>::of(&Clip3DWidget::update));
    timer_.start(16);
    elapsed_.start();
}

void Clip3DWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(3, 3, 8));

    const float t = static_cast<float>(elapsed_.elapsed()) / 1000.0f;
    const float ax = t * 0.9f;
    const float ay = t * 1.25f;
    const float az = t * 0.45f;
    const float scale = qMin(width(), height()) * 0.24f;
    const QPointF center(width() * 0.5, height() * 0.5);

    auto rotate = [=](Point3D p) {
        const float sx = qSin(ax), cx = qCos(ax);
        const float sy = qSin(ay), cy = qCos(ay);
        const float sz = qSin(az), cz = qCos(az);

        p = {p.x, p.y * cx - p.z * sx, p.y * sx + p.z * cx};
        p = {p.x * cy + p.z * sy, p.y, -p.x * sy + p.z * cy};
        p = {p.x * cz - p.y * sz, p.x * sz + p.y * cz, p.z};
        return p;
    };

    auto project = [=](Point3D p) {
        const float distance = 4.0f;
        const float perspective = distance / (distance - p.z);
        return QPointF(center.x() + p.x * scale * perspective,
                       center.y() + p.y * scale * perspective);
    };

    const QVector<Point3D> vertices = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1},
    };

    struct Face {
        int a;
        int b;
        int c;
        int d;
        QColor color;
    };

    QVector<Point3D> rotated;
    rotated.reserve(vertices.size());
    for (const Point3D &vertex : vertices) {
        rotated << rotate(vertex);
    }

    QList<Face> faces = {
        {0, 1, 2, 3, QColor(40, 170, 255)},
        {4, 7, 6, 5, QColor(255, 80, 70)},
        {3, 2, 6, 7, QColor(80, 245, 130)},
        {0, 4, 5, 1, QColor(255, 220, 60)},
        {1, 5, 6, 2, QColor(235, 80, 255)},
        {0, 3, 7, 4, QColor(60, 120, 255)},
    };

    std::sort(faces.begin(), faces.end(), [&](const Face &left, const Face &right) {
        const float zl = rotated[left.a].z + rotated[left.b].z + rotated[left.c].z + rotated[left.d].z;
        const float zr = rotated[right.a].z + rotated[right.b].z + rotated[right.c].z + rotated[right.d].z;
        return zl < zr;
    });

    painter.setPen(QPen(QColor(255, 255, 255, 190), 2));
    for (const Face &face : faces) {
        QPolygonF poly;
        poly << project(rotated[face.a])
             << project(rotated[face.b])
             << project(rotated[face.c])
             << project(rotated[face.d]);
        painter.setBrush(face.color);
        painter.drawPolygon(poly);
    }

    painter.setPen(QPen(QColor(255, 255, 255, 210), 1));
    painter.drawText(QRectF(0, height() - 44, width(), 28), Qt::AlignCenter, "Polo 3D clip - Esc to return");
}

void Clip3DWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit closeRequested();
        return;
    }
    QWidget::keyPressEvent(event);
}

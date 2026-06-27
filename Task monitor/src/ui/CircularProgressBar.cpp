#include "CircularProgressBar.h"
#include <QPainter>
#include <QPainterPath>
#include <QConicalGradient>
#include <QRadialGradient>
#include <cmath>

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent), value(0.0), barColor(QColor(160, 165, 175)) {}

void CircularProgressBar::setValue(double val) {
    if (val < 0.0) val = 0.0;
    if (val > 100.0) val = 100.0;
    if (std::abs(value - val) > 0.01) {
        value = val;
        update();
    }
}

void CircularProgressBar::setCustomText(const QString &text, const QString &subtext) {
    if (centerText != text || centerSubtext != subtext) {
        centerText = text;
        centerSubtext = subtext;
        update();
    }
}

void CircularProgressBar::setColor(const QColor &color) {
    barColor = color;
    update();
}

void CircularProgressBar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    int width = this->width();
    int height = this->height();
    int size = qMin(width, height) - 16; // Margin
    if (size <= 0) return;

    QRectF rect((width - size) / 2.0, (height - size) / 2.0, size, size);
    double strokeWidth = size * 0.1; // 10% of size
    if (strokeWidth < 4) strokeWidth = 4;

    // Adjusted rect for pen thickness
    QRectF arcRect = rect.adjusted(strokeWidth/2.0, strokeWidth/2.0, -strokeWidth/2.0, -strokeWidth/2.0);

    // 1. Draw Glassmorphic inner background circle
    QColor innerBgColor(42, 44, 48, 120);
    painter.setPen(Qt::NoPen);
    painter.setBrush(innerBgColor);
    painter.drawEllipse(arcRect);

    // 2. Draw Track (background ring)
    QPen trackPen;
    trackPen.setColor(QColor(70, 74, 82, 200));
    trackPen.setWidthF(strokeWidth);
    trackPen.setCapStyle(Qt::RoundCap);
    painter.setPen(trackPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(arcRect);

    // 3. Draw Glow behind active bar (subtle overlay)
    if (value > 0) {
        QPen glowPen;
        QColor glowColor = barColor;
        glowColor.setAlpha(40);
        glowPen.setColor(glowColor);
        glowPen.setWidthF(strokeWidth * 1.5);
        glowPen.setCapStyle(Qt::RoundCap);
        painter.setPen(glowPen);
        // Start angle: 90 degrees (top of circle). Span: clockwise negative direction
        double startAngle = 90.0;
        double spanAngle = -value * 3.6;
        painter.drawArc(arcRect, startAngle * 16, spanAngle * 16);

        // 4. Draw Active progress arc
        QPen activePen;
        // Gradient for a premium tech feel
        QLinearGradient grad(arcRect.topLeft(), arcRect.bottomRight());
        grad.setColorAt(0.0, barColor.lighter(120));
        grad.setColorAt(1.0, barColor);
        
        activePen.setBrush(grad);
        activePen.setWidthF(strokeWidth);
        activePen.setCapStyle(Qt::RoundCap);
        painter.setPen(activePen);
        painter.drawArc(arcRect, startAngle * 16, spanAngle * 16);
    }

    // 5. Draw center text
    // We compute font size relative to circle size
    double primaryFontSize = size * 0.16;
    double secondaryFontSize = size * 0.09;

    QFont font = painter.font();
    font.setFamily("Outfit"); // Premium font if available, otherwise system sans-serif
    font.setStyleHint(QFont::SansSerif);
    
    // Draw Main text
    font.setBold(true);
    font.setPointSizeF(primaryFontSize);
    painter.setFont(font);
    painter.setPen(QColor(240, 243, 248)); // Clean off-white

    if (centerSubtext.isEmpty()) {
        // Draw centered in the whole rect
        painter.drawText(rect, Qt::AlignCenter, centerText);
    } else {
        // Split vertical space for main text and subtext
        QRectF topRect = rect;
        topRect.setHeight(rect.height() * 0.6);
        topRect.moveTop(rect.top() + rect.height() * 0.12);
        
        painter.drawText(topRect, Qt::AlignCenter, centerText);

        // Draw Subtext
        font.setBold(false);
        font.setPointSizeF(secondaryFontSize);
        painter.setFont(font);
        painter.setPen(QColor(185, 187, 190)); // Slate silver-gray
        
        QRectF bottomRect = rect;
        bottomRect.setHeight(rect.height() * 0.4);
        bottomRect.moveTop(rect.top() + rect.height() * 0.52);
        painter.drawText(bottomRect, Qt::AlignCenter, centerSubtext);
    }
}

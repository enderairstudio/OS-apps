#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

#include <QWidget>
#include <QString>
#include <QColor>

class CircularProgressBar : public QWidget {
    Q_OBJECT
public:
    explicit CircularProgressBar(QWidget *parent = nullptr);

    void setValue(double val);
    void setCustomText(const QString &text, const QString &subtext = "");
    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double value;          // 0 to 100
    QString centerText;    // Custom text to show in the center
    QString centerSubtext; // Optional subtext (e.g. "GiB" or "Used")
    QColor barColor;       // Color of the progress arc
};

#endif // CIRCULARPROGRESSBAR_H

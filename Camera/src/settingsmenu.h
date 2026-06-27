#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include <QFrame>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class SettingsMenu : public QFrame {
    Q_OBJECT

public:
    explicit SettingsMenu(QWidget *parent = nullptr);

    int getTimerSeconds() const;
    int getFilterType() const;
    bool getRleEnabled() const;

signals:
    void timerChanged(int seconds);
    void filterChanged(int filterType);
    void compressionChanged(bool enabled);

private:
    void setupUi();

    QButtonGroup *timerGroup;
    QButtonGroup *filterGroup;
    QCheckBox *compressionCheck;

    int currentTimer;
    int currentFilter;
    bool currentRle;
};

#endif // SETTINGSMENU_H

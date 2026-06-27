#pragma once

#include "ui/TerminalTab.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class TopBar : public QWidget {
    Q_OBJECT

public:
    explicit TopBar(QWidget *parent = nullptr);

    void setTabs(const QList<TerminalTab *> &tabs, int activeIndex);

signals:
    void tabSelected(int index);
    void tabClosed(int index);
    void addTabRequested();

private:
    void rebuild();

    QList<TerminalTab *> tabs_;
    int activeIndex_ = 0;
    QHBoxLayout *layout_ = nullptr;
};

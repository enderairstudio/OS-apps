#pragma once

#include "render/Clip3DWidget.h"
#include "ui/TerminalTab.h"
#include "ui/TerminalView.h"
#include "ui/TopBar.h"

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

class TerminalWindow : public QWidget {
    Q_OBJECT

public:
    explicit TerminalWindow(QWidget *parent = nullptr);
    ~TerminalWindow() override;

private:
    void addTab();
    void closeTab(int index);
    void setActiveTab(int index);
    void refreshChrome();
    void show3D();
    void hide3D();

    QList<TerminalTab *> tabs_;
    int activeIndex_ = -1;

    TopBar *topBar_ = nullptr;
    QLabel *pathLabel_ = nullptr;
    QStackedWidget *stack_ = nullptr;
    TerminalView *terminalView_ = nullptr;
    Clip3DWidget *clip3D_ = nullptr;
};

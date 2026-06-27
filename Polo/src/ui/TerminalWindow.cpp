#include "TerminalWindow.h"

#include <QApplication>

TerminalWindow::TerminalWindow(QWidget *parent)
    : QWidget(parent),
      topBar_(new TopBar(this)),
      pathLabel_(new QLabel(this)),
      stack_(new QStackedWidget(this)),
      terminalView_(new TerminalView(this)),
      clip3D_(new Clip3DWidget(this))
{
    setWindowTitle("Polo");

    pathLabel_->setObjectName("pathLabel");
    pathLabel_->setFixedHeight(28);
    pathLabel_->setContentsMargins(12, 0, 12, 0);

    stack_->addWidget(terminalView_);
    stack_->addWidget(clip3D_);
    stack_->setCurrentWidget(terminalView_);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(topBar_);
    layout->addWidget(pathLabel_);
    layout->addWidget(stack_, 1);

    connect(topBar_, &TopBar::addTabRequested, this, &TerminalWindow::addTab);
    connect(topBar_, &TopBar::tabSelected, this, &TerminalWindow::setActiveTab);
    connect(topBar_, &TopBar::tabClosed, this, &TerminalWindow::closeTab);
    connect(terminalView_, &TerminalView::show3DRequested, this, &TerminalWindow::show3D);
    connect(terminalView_, &TerminalView::closeTabRequested, this, [this]() { closeTab(activeIndex_); });
    connect(terminalView_, &TerminalView::tabStateChanged, this, &TerminalWindow::refreshChrome);
    connect(clip3D_, &Clip3DWidget::closeRequested, this, &TerminalWindow::hide3D);

    addTab();
}

TerminalWindow::~TerminalWindow()
{
    qDeleteAll(tabs_);
}

void TerminalWindow::addTab()
{
    TerminalTab *tab = new TerminalTab(this);
    tabs_ << tab;
    connect(tab, &TerminalTab::changed, this, &TerminalWindow::refreshChrome);
    setActiveTab(tabs_.size() - 1);
}

void TerminalWindow::closeTab(int index)
{
    if (index < 0 || index >= tabs_.size()) {
        return;
    }
    if (tabs_.size() == 1) {
        qApp->quit();
        return;
    }

    TerminalTab *tab = tabs_.takeAt(index);
    tab->deleteLater();

    if (activeIndex_ >= tabs_.size()) {
        activeIndex_ = tabs_.size() - 1;
    }
    setActiveTab(activeIndex_);
}

void TerminalWindow::setActiveTab(int index)
{
    if (index < 0 || index >= tabs_.size()) {
        return;
    }
    activeIndex_ = index;
    hide3D();
    terminalView_->attachTab(tabs_.at(activeIndex_));
    refreshChrome();
    terminalView_->setFocus();
}

void TerminalWindow::refreshChrome()
{
    if (activeIndex_ < 0 || activeIndex_ >= tabs_.size()) {
        return;
    }
    TerminalTab *tab = tabs_.at(activeIndex_);
    topBar_->setTabs(tabs_, activeIndex_);
    pathLabel_->setText(tab->displayDirectory());
}

void TerminalWindow::show3D()
{
    stack_->setCurrentWidget(clip3D_);
    pathLabel_->clear();
    clip3D_->setFocus();
}

void TerminalWindow::hide3D()
{
    if (stack_->currentWidget() != terminalView_) {
        stack_->setCurrentWidget(terminalView_);
    }
    if (activeIndex_ >= 0 && activeIndex_ < tabs_.size()) {
        pathLabel_->setText(tabs_.at(activeIndex_)->displayDirectory());
    }
    terminalView_->setFocus();
}

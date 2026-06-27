#include "TopBar.h"

#include <QLabel>
#include <QIcon>

TopBar::TopBar(QWidget *parent)
    : QWidget(parent),
      layout_(new QHBoxLayout(this))
{
    setFixedHeight(44);
    layout_->setContentsMargins(8, 6, 8, 4);
    layout_->setSpacing(6);
}

void TopBar::setTabs(const QList<TerminalTab *> &tabs, int activeIndex)
{
    tabs_ = tabs;
    activeIndex_ = activeIndex;
    rebuild();
}

void TopBar::rebuild()
{
    while (QLayoutItem *item = layout_->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    for (int i = 0; i < tabs_.size(); ++i) {
        QWidget *tabShell = new QWidget(this);
        QHBoxLayout *tabLayout = new QHBoxLayout(tabShell);
        tabLayout->setContentsMargins(0, 0, 0, 0);
        tabLayout->setSpacing(4);

        QPushButton *tabButton = new QPushButton(tabs_.at(i)->title(), tabShell);
        tabButton->setIcon(QIcon(":/polo/logo.svg"));
        tabButton->setIconSize(QSize(22, 22));
        tabButton->setProperty("active", i == activeIndex_);
        tabButton->setMinimumWidth(180);
        tabButton->setMaximumWidth(320);
        tabButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        connect(tabButton, &QPushButton::clicked, this, [this, i]() { emit tabSelected(i); });
        tabLayout->addWidget(tabButton);

        QPushButton *closeButton = new QPushButton("x", tabShell);
        closeButton->setObjectName("closeButton");
        closeButton->setToolTip("Close tab");
        connect(closeButton, &QPushButton::clicked, this, [this, i]() { emit tabClosed(i); });
        tabLayout->addWidget(closeButton);

        layout_->addWidget(tabShell);
    }

    QPushButton *addButton = new QPushButton("+", this);
    addButton->setObjectName("addButton");
    addButton->setToolTip("New tab");
    connect(addButton, &QPushButton::clicked, this, &TopBar::addTabRequested);
    layout_->addWidget(addButton);
    layout_->addStretch(1);
}

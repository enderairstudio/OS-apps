#include "settingsmenu.h"
#include <QPushButton>

SettingsMenu::SettingsMenu(QWidget *parent)
    : QFrame(parent), currentTimer(0), currentFilter(0), currentRle(true) {
    
    // Set window properties for a borderless popup
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    setupUi();
}

void SettingsMenu::setupUi() {
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // Frame styling
    setStyleSheet(
        "QFrame {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #252528, stop:1 #1a1a1c);"
        "  border: 1px solid #3c3c40;"
        "  border-radius: 12px;"
        "}"
        "QLabel {"
        "  color: #a0a0a5;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton {"
        "  color: #e0e0e0;"
        "  background-color: #2f2f33;"
        "  border: 1px solid #3c3c40;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3e3e44;"
        "  border-color: #5c5c62;"
        "}"
        "QPushButton:checked {"
        "  color: #ffffff;"
        "  background-color: #007acc;"
        "  border-color: #0098ff;"
        "  font-weight: bold;"
        "}"
        "QCheckBox {"
        "  color: #e0e0e0;"
        "  font-size: 12px;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QCheckBox::indicator {"
        "  width: 16px;"
        "  height: 16px;"
        "  border: 1px solid #4c4c52;"
        "  border-radius: 4px;"
        "  background-color: #2f2f33;"
        "}"
        "QCheckBox::indicator:hover {"
        "  border-color: #6c6c74;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: #007acc;"
        "  border-color: #0098ff;"
        "  image: url(dummy); /* Force showing checkmark check style standard */"
        "}"
    );

    // Title/Header
    QLabel *titleLabel = new QLabel("CAMERA OPTIONS", this);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: bold; margin-bottom: 4px;");
    mainLayout->addWidget(titleLabel);

    // --- Timer Section ---
    QLabel *timerLabel = new QLabel("TIMER", this);
    mainLayout->addWidget(timerLabel);

    QHBoxLayout *timerLayout = new QHBoxLayout();
    timerLayout->setSpacing(6);
    timerGroup = new QButtonGroup(this);
    timerGroup->setExclusive(true);

    struct TimerOpt {
        QString text;
        int seconds;
    } timerOpts[] = {
        {"Off", 0},
        {"2s", 2},
        {"5s", 5},
        {"10s", 10}
    };

    for (int i = 0; i < 4; ++i) {
        QPushButton *btn = new QPushButton(timerOpts[i].text, this);
        btn->setCheckable(true);
        if (timerOpts[i].seconds == currentTimer) {
            btn->setChecked(true);
        }
        timerLayout->addWidget(btn);
        timerGroup->addButton(btn, timerOpts[i].seconds);
    }
    mainLayout->addLayout(timerLayout);

    connect(timerGroup, &QButtonGroup::idClicked, this, [this](int seconds) {
        currentTimer = seconds;
        emit timerChanged(seconds);
    });

    // --- Filters Section ---
    QLabel *filtersLabel = new QLabel("COLOR FILTER", this);
    mainLayout->addWidget(filtersLabel);

    QGridLayout *filterLayout = new QGridLayout();
    filterLayout->setSpacing(6);
    filterGroup = new QButtonGroup(this);
    filterGroup->setExclusive(true);

    struct FilterOpt {
        QString text;
        int id;
    } filterOpts[] = {
        {"Normal", 0},
        {"Mono", 1},
        {"Sepia", 2},
        {"Invert", 3},
        {"Warm", 4},
        {"Cool", 5}
    };

    for (int i = 0; i < 6; ++i) {
        QPushButton *btn = new QPushButton(filterOpts[i].text, this);
        btn->setCheckable(true);
        if (filterOpts[i].id == currentFilter) {
            btn->setChecked(true);
        }
        filterLayout->addWidget(btn, i / 3, i % 3);
        filterGroup->addButton(btn, filterOpts[i].id);
    }
    mainLayout->addLayout(filterLayout);

    connect(filterGroup, &QButtonGroup::idClicked, this, [this](int id) {
        currentFilter = id;
        emit filterChanged(id);
    });

    // --- Compression Section ---
    QLabel *compLabel = new QLabel("COMPRESSION", this);
    mainLayout->addWidget(compLabel);

    compressionCheck = new QCheckBox("Run-Length Encoding (.pff RLE)", this);
    compressionCheck->setChecked(currentRle);
    mainLayout->addWidget(compressionCheck);

    connect(compressionCheck, &QCheckBox::toggled, this, [this](bool checked) {
        currentRle = checked;
        emit compressionChanged(checked);
    });
}

int SettingsMenu::getTimerSeconds() const { return currentTimer; }
int SettingsMenu::getFilterType() const { return currentFilter; }
bool SettingsMenu::getRleEnabled() const { return currentRle; }

#include <QApplication>
#include "ui/MainWindow.h"
#include <cstdlib>
#include <ctime>

int main(int argc, char *argv[]) {
    // Seed random generator for simulated values fallback
    std::srand(std::time(nullptr));

    QApplication app(argc, argv);
    
    MainWindow w;
    w.show();
    
    return app.exec();
}

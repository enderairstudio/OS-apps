#include <QApplication>
#include "gallerywindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    GalleryWindow w;
    w.show();
    
    return app.exec();
}

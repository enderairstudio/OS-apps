#include "ui/TerminalWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QIcon>
#include <QSurfaceFormat>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    QApplication::setApplicationName("Polo");
    QApplication::setOrganizationName("Polo");

    app.setWindowIcon(QIcon(":/polo/logo.svg"));

    QFile theme(":/polo/theme.qss");
    if (theme.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&theme);
        app.setStyleSheet(stream.readAll());
    }

    TerminalWindow window;
    window.resize(1100, 720);
    window.show();

    return app.exec();
}

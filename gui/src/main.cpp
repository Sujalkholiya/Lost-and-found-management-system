#include <QApplication>
#include <QFont>
#include <QDebug>
#include "views/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("LostAndFoundGUI");
    app.setOrganizationName("CollegeLostAndFound");
    app.setApplicationDisplayName("Campus Lost & Found");

    // Modern clean typography
    QFont defaultFont("Segoe UI", 10);
    defaultFont.setStyleHint(QFont::SansSerif);
    app.setFont(defaultFont);

    qDebug() << "[Main] Starting Campus Lost & Found Qt GUI Application...";

    MainWindow window;
    window.show();

    qDebug() << "[Main] Application event loop started.";

    return app.exec();
}
#include "gui/mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Fuzzy Droplets");
    a.setApplicationDisplayName("Fuzzy Droplets");
    a.setOrganizationName("Fuzzy Droplets");
    a.setOrganizationDomain("fuzzydroplets.com");

#ifdef Q_OS_WIN
    a.setStyle("fusion"); // better for dark themes
#endif

    MainWindow w;
    w.show();
    return a.exec();
}

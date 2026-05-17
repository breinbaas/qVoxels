#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    //qputenv("PROJ_DATA", (QCoreApplication::applicationDirPath() + "/proj_data").toUtf8());
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}

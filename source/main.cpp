#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_USE_ANDROID_NATIVE_STYLE", "1");
    //qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

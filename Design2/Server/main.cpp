#include "mainwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QByteArray width = qgetenv("QT_QPA_EGLFS_PYSICAL_WIDTH");
    width.setNum(100);
    QByteArray height = qgetenv("QT_QPA_EGLFS_PYSICAL_HEIGHT");
    height.setNum(100);
    QApplication a(argc, argv);
    MainWidget w;
    w.show();

    return a.exec();
}

#include "mainwidget.h"
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDialog>

int main(int argc, char *argv[])
{
    QApplication b(argc, argv);
    MainWidget w;
    w.show();

    return b.exec();
}

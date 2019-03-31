#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleFile(":/stylesheet.qss");
    if (styleFile.open(QFile::ReadOnly))
    {
        QString style(styleFile.readAll());
        a.setStyleSheet(style);
    }

    MainWindow w;
    w.show();

    return a.exec();
}

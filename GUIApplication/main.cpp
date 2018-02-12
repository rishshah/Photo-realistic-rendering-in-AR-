#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSurfaceFormat fmt;
    fmt.setVersion( 4, 0 );
    fmt.setProfile( QSurfaceFormat::CoreProfile );
    QSurfaceFormat::setDefaultFormat( fmt );
    MainWindow w;
    w.show();

    return a.exec();
}

#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QRect>
#include <QScreen>

#include "cslideshow.h"
#include "cmainwindow.h"
#include "server.h"

#define VERSION "v001b"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CMainWindow* w = new CMainWindow(VERSION);
    w->show();

//    Server server;
//    server.show();

    return a.exec();
}

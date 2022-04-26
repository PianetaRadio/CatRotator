#include "mainwindow.h"
#include "rotudp.h"
#include "rotatordata.h"

#include <QApplication>

extern rotatorConfig rotCfg;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //* UDP
    rotUdp listenerUdp;
    if (rotCfg.udp) listenerUdp.initSocket();

    return a.exec();
}

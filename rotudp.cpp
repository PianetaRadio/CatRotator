#include "rotudp.h"
#include "rotatordata.h"
#include "mainwindow.h"

#include <QRegularExpression>


extern rotatorConfig rotCfg;
extern rotatorUdpEx rotUdpEx;


rotUdp::rotUdp(QObject *parent)
    : QObject{parent}
{

}

bool rotUdp::initSocket()
{
    udpSocket = new QUdpSocket(this);
    bool result = udpSocket->bind(QHostAddress(rotCfg.udpAddress), rotCfg.udpPort);

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));

    return result;
}

void rotUdp::readDatagrams()
{
    QHostAddress sender;
    quint16 senderPort;

    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagrams;
        datagrams.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagrams.data(), datagrams.size(), &sender, &senderPort);

        //qDebug() << "Message from: " << sender.toString();
        //qDebug() << "Message port: " << senderPort;
        qDebug() << "Message: " << datagrams;

        //QRegularExpression pstAzimuth("<PST><AZIMUTH>(\\d+)</AZIMUTH></PST>");
        QRegularExpression pstAzimuth("<AZIMUTH>(\\d+)</AZIMUTH>");
        QRegularExpressionMatch azMatch = pstAzimuth.match(datagrams);
        if (azMatch.hasMatch())
        {
            QString az = azMatch.captured(1);
            rotUdpEx.azUdpFlag = true;
            rotUdpEx.azUdp = az.toInt();
        }
    }
}

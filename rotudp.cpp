/**
 ** This file is part of the CatRotator project.
 ** Copyright 2022 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/


#include "rotudp.h"
#include "rotatordata.h"
#include "mainwindow.h"

#include <QRegularExpression>


extern catRotatorConfig rotCfg;
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
        //qDebug() << "Message: " << datagrams;

        //QRegularExpression pstAzimuth("<PST><AZIMUTH>(\\d+)</AZIMUTH></PST>");
        QRegularExpression pstAzCmd("<AZIMUTH>(\\d+\\.?\\d*)</AZIMUTH>");
        QRegularExpressionMatch pstMatch = pstAzCmd.match(datagrams);
        if (pstMatch.hasMatch())
        {
            QString pstMatchString = pstMatch.captured(1);
            rotUdpEx.azUdpFlag = true;
            rotUdpEx.azUdp = pstMatchString.toFloat();
        }

        QRegularExpression pstElCmd("<ELEVATION>(\\d+\\.?\\d*)</ELEVATION>");
        pstMatch = pstElCmd.match(datagrams);
        if (pstMatch.hasMatch())
        {
            QString pstMatchString = pstMatch.captured(1);
            rotUdpEx.elUdpFlag = true;
            rotUdpEx.elUdp = pstMatchString.toFloat();
        }

        QRegularExpression pstStopCmd("<STOP>(\\d)</STOP>");
        pstMatch = pstStopCmd.match(datagrams);
        if (pstMatch.hasMatch())
        {
            QString pstMatchString = pstMatch.captured(1);
            rotUdpEx.stopUdpFlag = pstMatchString.toInt();
        }

        QRegularExpression pstParkCmd("<PARK>(\\d)</PARK>");
        pstMatch = pstParkCmd.match(datagrams);
        if (pstMatch.hasMatch())
        {
            QString pstMatchString = pstMatch.captured(1);
            rotUdpEx.parkUdpFlag = pstMatchString.toInt();
        }
    }
}

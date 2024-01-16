/**
 ** This file is part of the CatRotator project.
 ** Copyright 2022-2024 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
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
//#include "mainwindow.h"

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

        //N1MM Logger+
        QRegularExpression n1mmCmd("<N1MMRotor>");
        QRegularExpressionMatch n1mmMatch = n1mmCmd.match(datagrams);
        if (n1mmMatch.hasMatch())
        {
            rotUdpEx.n1mmUdp = true;

            QRegularExpression n1mmName("<rotor>(.+)</rotor>");
            n1mmMatch = n1mmName.match(datagrams);
            if (n1mmMatch.hasMatch())
            {
                QString n1mmMatchString = n1mmMatch.captured(1);
                rotUdpEx.rotName = n1mmMatchString;
            }

            QRegularExpression n1mmAzCmd("<goazi>(\\d+\\,?\\.?\\d*)</goazi>");
            QRegularExpressionMatch n1mmMatch = n1mmAzCmd.match(datagrams);
            if (n1mmMatch.hasMatch())
            {
                QString n1mmMatchString = n1mmMatch.captured(1);
                n1mmMatchString.replace(",", ".", Qt::CaseInsensitive);
                rotUdpEx.azUdpFlag = true;
                rotUdpEx.azUdp = n1mmMatchString.toFloat();
            }

            QRegularExpression n1mmStopCmd("<stop>");
            QRegularExpressionMatch n1mmStop = n1mmStopCmd.match(datagrams);
            if (n1mmStop.hasMatch()) rotUdpEx.stopUdpFlag = true;
        }

        //Previsat
        QRegularExpression previSatCmd("<PREVISAT>");
        QRegularExpressionMatch previMatch = previSatCmd.match(datagrams);
        if (previMatch.hasMatch())
        {
            rotUdpEx.previSatUdp = true;

            QRegularExpression previSatName("<SAT>(.+)</SAT>");
            previMatch = previSatName.match(datagrams);
            if (previMatch.hasMatch())
            {
                QString previMatchString = previMatch.captured(1);
                rotUdpEx.satName = previMatchString;
            }

            QRegularExpression previSatAOS("<AOS>(\\d)</AOS>");
            previMatch = previSatAOS.match(datagrams);
            if (previMatch.hasMatch())
            {
                QString previMatchString = previMatch.captured(1);
                rotUdpEx.satAOS = previMatchString.toUInt();
            }
        }

        QRegularExpression pstAzCmd("<AZIMUTH>(\\d+\\.?\\d*)</AZIMUTH>");
        QRegularExpressionMatch pstMatch = pstAzCmd.match(datagrams);
        if (pstMatch.hasMatch())
        {
            QString pstMatchString = pstMatch.captured(1);
            rotUdpEx.azUdpFlag = true;
            rotUdpEx.azUdp = pstMatchString.toFloat();
            //rotUdpEx.elUdp = 0;
        }

        QRegularExpression pstElCmd("<ELEVATION>(-?\\d+\\.?\\d*)</ELEVATION>");
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

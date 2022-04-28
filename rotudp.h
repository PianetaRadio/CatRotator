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


#ifndef ROTUDP_H
#define ROTUDP_H

#include <QObject>
#include <QUdpSocket>

class rotUdp : public QObject
{
    Q_OBJECT

public:
    explicit rotUdp(QObject *parent = nullptr);
    bool initSocket();

    bool azUdpFlag;
    int azUdp;

private slots:
    void readDatagrams();

private:
    QUdpSocket *udpSocket;

signals:
    void resultReady();

};

#endif // ROTUDP_H

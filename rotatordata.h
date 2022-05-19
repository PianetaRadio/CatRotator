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


#ifndef ROTATORDATA_H
#define ROTATORDATA_H

#include <rotator.h>

#include <QString>


typedef struct {
    unsigned rotModel;  //Hamlib rig model
    QString rotPort;    //COM port or IP address
    unsigned serialSpeed;   //Serial port baud rate
    bool netRotctl; //TCP NET Rotctl
    int connected;  //connected flag
} rotatorConnect;


typedef struct {
    bool enable;
    QString nameLabel;
    azimuth_t az, azPark;
    elevation_t el, elPark;
} rotatorSettings;


typedef struct {
    unsigned rotRefresh;    //GUI refresh interval (ms)
    QString qthLocator; //QTH WW Locator
    bool udp;   //UDP enable flag
    QString udpAddress; //UDP address
    quint16 udpPort;    //UDP port
    int preset[9];  //Preset button array
} catRotatorConfig;


typedef struct {
    bool azUdpFlag, elUdpFlag, stopUdpFlag, parkUdpFlag;
    int azUdp, elUdp;
} rotatorUdpEx;

#endif // ROTATORDATA_H

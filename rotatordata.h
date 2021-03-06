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
    bool enable;    //Rotor enabled
    QString nameLabel;  //Rotor name
    azimuth_t az, azPark;
    elevation_t el, elPark;
    bool lPathFlag;    //Long Path
} rotatorSettings;


typedef struct {
    unsigned rotRefresh;    //GUI refresh interval (ms)
    QString qthLocator; //QTH WW Locator
    bool udp;   //UDP enable flag
    QString udpAddress; //UDP address
    quint16 udpPort;    //UDP port
    QString presetLabel[9]; //Preset button array label
    int presetAz[9];  //Preset button array azimuth
} catRotatorConfig;


typedef struct {
    bool azUdpFlag, elUdpFlag, stopUdpFlag, parkUdpFlag;    //UDP received command
    int azUdp, elUdp;
} rotatorUdpEx;

#endif // ROTATORDATA_H

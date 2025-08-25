/**
 ** This file is part of the CatRotator project.
 ** Copyright 2022-2025 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
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
    azimuth_t az, azPark, azOffset;
    elevation_t el, elPark, elOffset;
    bool overlap;   //Overlap capacity
    bool lPathFlag;    //Long Path
    bool trackFlag, trackPreviSat, trackAirScout; //Tracking
    int trackWSJTX;
    double trackTolerance;    //Tracking tolerance
    double trackThreshold;  //Tracking elevation threshold
} rotatorSettings;


typedef struct {
    unsigned rotRefresh;    //GUI refresh interval (ms)
    QString qthLocator; //QTH WW Locator
    bool distance;  //unit for distance: 0 kilometers, 1 miles
    bool udp;   //UDP enable flag
    QString udpAddress; //UDP address
    quint16 udpPort;    //UDP port
    QString presetLabel[9]; //Preset button array label
    int presetAz[9];  //Preset button array azimuth
    int incrementAz; //Increment interval for plus/minus buttons;
    QString pathTrackWSJTXStatus;   //Path for WSJT-X Status file;
    QString pathTrackWSJTX; //Path for WSJT-X AzEl file;
    QString pathTrackAirScout;  //Path for AirScout tracking file;
    bool darkTheme; //flag for Dark theme
    bool debugMode; //flag for debug log
    bool autoConnect;   //flag for auto-connect
} catRotatorConfig;


typedef struct {
    bool azUdpFlag, elUdpFlag, stopUdpFlag, parkUdpFlag;    //UDP received command
    bool previSatUdp;   //PreviSat UDP datagrams
    bool n1mmUdp;   //N1MM Logger+ datagrams
    double azUdp, elUdp;
    QString satName, rotName;
    bool satAOS;
} rotatorUdpEx;

#endif // ROTATORDATA_H

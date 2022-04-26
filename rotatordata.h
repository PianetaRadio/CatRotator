#ifndef ROTATORDATA_H
#define ROTATORDATA_H

#include <rotator.h>

#include <QString>


typedef struct {
    unsigned rotModel;  //Hamlib rig model
    QString rotPort;    //COM port or IP address
    unsigned serialSpeed;   //Serial port baud rate
    bool netRotctl; //TCP NET Rotctl
    unsigned rotRefresh;    //GUI refresh interval (ms)
    int connected;  //connected flag
} rotatorConnect;


typedef struct {
    azimuth_t az, azPark;
    elevation_t el, elPark;
} rotatorSettings;


typedef struct {
    bool udp;   //UDP enable flag
    QString udpAddress;
    quint16 udpPort;
} rotatorConfig;

typedef struct {
    bool azUdpFlag;
    int azUdp;
} rotatorUdpEx;


#endif // ROTATORDATA_H

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

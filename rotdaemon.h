#ifndef ROTDAEMON_H
#define ROTDAEMON_H

#include <QObject>
#include <rotator.h>

class RotDaemon : public QObject
{
    Q_OBJECT

public:
    explicit RotDaemon(QObject *parent = nullptr);
    int rotConnect();

public slots:
    void rotUpdate();

signals:
    void resultReady();

};

#endif // ROTDAEMON_H

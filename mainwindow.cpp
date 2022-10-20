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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogrotator.h"
#include "dialogsetup.h"
#include "dialogpreset.h"
#include "rotatordata.h"
#include "rotdaemon.h"

#include <QMessageBox>
#include <QThread>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QRegularExpression>
#include <QDir>
#include <QFile>

#include <rotator.h>    //Hamlib

ROT *my_rot;
ROT *my_rot2;
ROT *my_rot3;

extern rotatorConnect rotCom[3];
extern rotatorSettings rotGet[3];
extern rotatorSettings rotSet[3];
extern catRotatorConfig rotCfg;
extern rotatorUdpEx rotUdpEx;

int retcode;    //Return code from function
int defaultPreset[9] = {0, 45, 90, 135, 180, 225, 270, 315, 360};

FILE* debugFile;

QThread workerThread;
RotDaemon *rotDaemon = new RotDaemon;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    ui->tabWidget_rotator->setTabVisible(1, false);
    ui->tabWidget_rotator->setTabVisible(2, false);
    ui->tabWidget_rotator->setTabVisible(3, false);
#endif

    timer = new QTimer(this);   //timer for rotDaemon thread call

    //* Debug
    rig_set_debug_level(RIG_DEBUG_WARN);  //normal
    //rig_set_debug_level(RIG_DEBUG_TRACE);   //debug
    rig_set_debug_time_stamp(true);
    if ((debugFile=fopen("catrotator.log","w+")) == NULL) rig_set_debug_level(RIG_DEBUG_NONE);
    else rig_set_debug_file(debugFile);

    //* Thread for RigDaemon
    rotDaemon->moveToThread(&workerThread); //
    connect(&workerThread, &QThread::finished, rotDaemon, &QObject::deleteLater);
    connect(timer, &QTimer::timeout, this, &MainWindow::rotUpdate);
    connect(rotDaemon, &RotDaemon::resultReady, this, &MainWindow::on_rotDaemonResultReady);
    workerThread.start();

    //* Load settings from catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);

    for (int i = 0; i < 3; i++)
    {
        rotCom[i].rotModel = configFile.value("Rotator"+QString::number(i+1)+"/rotModel", 0).toInt();
        rotCom[i].rotPort = configFile.value("Rotator"+QString::number(i+1)+"/rotPort").toString();
        rotCom[i].serialSpeed = configFile.value("Rotator"+QString::number(i+1)+"/serialSpeed", 9600).toInt();
        rotCom[i].netRotctl = configFile.value("Rotator"+QString::number(i+1)+"/netRotctl", false).toBool();
        if (rotCom[i].rotModel) rotSet[i].enable = true;
        rotSet[i].nameLabel = configFile.value("Rotator"+QString::number(i+1)+"/nameLabel", "Rotator "+QString::number(i+1)).toString();
        rotSet[i].azPark = configFile.value("Rotator"+QString::number(i+1)+"/azPark", 0).toInt();
        rotSet[i].elPark = configFile.value("Rotator"+QString::number(i+1)+"/elPark", 0).toInt();
        rotSet[i].overlap = configFile.value("Rotator"+QString::number(i+1)+"/overlap", false).toBool();
        rotSet[i].trackTolerance = configFile.value("Rotator"+QString::number(i+1)+"/trackTolerance", 5.0).toDouble();
        rotSet[i].trackThreshold = configFile.value("Rotator"+QString::number(i+1)+"/trackThreshold", 0.0).toDouble();
        rotSet[i].trackPreviSat = configFile.value("Rotator"+QString::number(i+1)+"/trackPreviSat", false).toBool();
        rotSet[i].trackWSJTX = configFile.value("Rotator"+QString::number(i+1)+"/trackWSJTX", false).toBool();
        rotSet[i].trackAirScout = configFile.value("Rotator"+QString::number(i+1)+"/trackAirScout", false).toBool();
    }

    rotCfg.rotRefresh = configFile.value("rotRefresh", 1).toInt();
    rotCfg.incrementAz = configFile.value("rotIncrementAz", 10).toInt();
    rotCfg.qthLocator = configFile.value("qthLocator", "").toString();
    rotCfg.udp = configFile.value("udp", false).toBool();
    rotCfg.udpAddress = configFile.value("udpAddress", "127.0.0.1").toString();
    rotCfg.udpPort = configFile.value("udpPort", 12000).toUInt();   //should be toUShort()
    rotCfg.pathTrackWSJTX = configFile.value("pathTrackWSJTX", QDir::homePath() + "/AppData/Local/WSJT-X").toString();      //RPi /home/pi/.local/share/WSJT-X
    rotCfg.pathTrackAirScout = configFile.value("pathTrackAirScout", QDir::homePath() + "/AppData/Local/DL2ALF/AirScout/Tmp").toString();

    //Presets
    MainWindow::presetInit();

    //Window settings
    restoreGeometry(configFile.value("WindowSettings/geometry").toByteArray());
    restoreState(configFile.value("WindowSettings/state").toByteArray());
}

MainWindow::~MainWindow()
{
    workerThread.quit();
    workerThread.wait();

    if (rotCom[0].connected || rotCom[1].connected || rotCom[2].connected) timer->stop();

    if (rotCom[0].connected)
    {
        rotCom[0].connected = 0;
        rot_close(my_rot);  //Close the communication to the rotator
        rot_cleanup(my_rot);    //Release rot handle and free associated memory
    }

    if (rotCom[1].connected)
    {
        rotCom[1].connected = 0;
        rot_close(my_rot2);
        rot_cleanup(my_rot2);
    }

    if (rotCom[2].connected)
    {
        rotCom[2].connected = 0;
        rot_close(my_rot3);
        rot_cleanup(my_rot3);
    }

    //* Save window settings
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    configFile.setValue("WindowSettings/geometry", saveGeometry());
    configFile.setValue("WindowSettings/state", saveState());

    delete ui;
}

//* Get values
void MainWindow::rotUpdate()
{
    if (rotCom[0].connected) rotDaemon->rotUpdate(0, my_rot, &rotGet[0]);
    if (rotSet[1].enable && rotCom[1].connected) rotDaemon->rotUpdate(1, my_rot2, &rotGet[1]);
    if (rotSet[2].enable && rotCom[2].connected) rotDaemon->rotUpdate(2, my_rot3, &rotGet[2]);
}

//* RotDaemon handle results
void MainWindow::on_rotDaemonResultReady(int rotNumber)
{
    guiUpdate(rotNumber);
}

void MainWindow::guiInit()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ui->tabWidget_rotator->removeTab(3);
#endif

    if (rotSet[2].enable)
    {
        ui->tabWidget_rotator->setTabText(2, rotSet[2].nameLabel);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ui->tabWidget_rotator->setTabVisible(2, true);
#endif
        if (rotCom[2].connected)
        {
            ui->tabWidget_rotator->setTabEnabled(2, true);
            if (my_rot3->caps->rot_type == ROT_TYPE_AZIMUTH) ui->lcdNumber_posEl_3->setVisible(false);
            if (my_rot3->caps->rot_type == ROT_TYPE_ELEVATION) ui->toolButton_pathSL_3->setVisible(false);
        }
        else ui->tabWidget_rotator->setTabEnabled(2, false);
    }
    else
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ui->tabWidget_rotator->setTabVisible(2, false);
#else
    ui->tabWidget_rotator->removeTab(2);
#endif
    }

    if (rotSet[1].enable)
    {
        ui->tabWidget_rotator->setTabText(1, rotSet[1].nameLabel);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ui->tabWidget_rotator->setTabVisible(1, true);
#endif
        if (rotCom[1].connected)
        {
            ui->tabWidget_rotator->setTabEnabled(1, true);
            if (my_rot2->caps->rot_type == ROT_TYPE_AZIMUTH) ui->lcdNumber_posEl_2->setVisible(false);
            if (my_rot2->caps->rot_type == ROT_TYPE_ELEVATION) ui->toolButton_pathSL_2->setVisible(false);
        }
        else ui->tabWidget_rotator->setTabEnabled(1, false);
    }
    else
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ui->tabWidget_rotator->setTabVisible(1, false);
#else
    ui->tabWidget_rotator->removeTab(1);
#endif
    }

    ui->tabWidget_rotator->setTabText(0, rotSet[0].nameLabel);
    if (rotCom[0].connected)
    {
        ui->tabWidget_rotator->setTabEnabled(0, true);
        if (my_rot->caps->rot_type == ROT_TYPE_AZIMUTH) ui->lcdNumber_posEl->setVisible(false);
        if (my_rot->caps->rot_type == ROT_TYPE_ELEVATION) ui->toolButton_pathSL->setVisible(false);
        //ui->spinBox_posAz->setMaximum(my_rot->caps->max_az);
        //ui->spinBox_posAz->setMinimum(my_rot->caps->min_az);
    }
    else ui->tabWidget_rotator->setTabEnabled(0, false);
}

void MainWindow::guiUpdate(int rotNumber)
{
    //Update current position
    if (rotSet[0].enable)
    {
        ui->lcdNumber_posAz->display(QString::number(rotGet[0].az, 'f', 1));
        ui->lcdNumber_posEl->display(QString::number(rotGet[0].el, 'f', 1));
    }

    if (rotSet[1].enable)
    {
        ui->lcdNumber_posAz_2->display(QString::number(rotGet[1].az, 'f', 1));
        ui->lcdNumber_posEl_2->display(QString::number(rotGet[1].el, 'f', 1));
    }

    if (rotSet[2].enable)
    {
        ui->lcdNumber_posAz_3->display(QString::number(rotGet[2].az, 'f', 1));
        ui->lcdNumber_posEl_3->display(QString::number(rotGet[2].el, 'f', 1));
    }

    //Parse UDP command
    if ((rotUdpEx.azUdpFlag || rotUdpEx.elUdpFlag) && (!rotUdpEx.previSatUdp))
    {
        rotUdpEx.azUdpFlag = false;
        rotUdpEx.elUdpFlag = false;

        switch (ui->tabWidget_rotator->currentIndex())
        {
        case 0:
            //rotSet[0].az = rotUdpEx.azUdp;
            //rotSet[0].el = rotUdpEx.elUdp;
            setPosition(0, rotUdpEx.azUdp, rotUdpEx.elUdp);
            ui->lineEdit_posAz->setText(QString::number(rotSet[0].az, 'f', 1) + " " + QString::number(rotSet[0].el, 'f', 1));
            //rot_set_position(my_rot, rotSet[0].az, rotSet[0].el);
            break;
        case 1:
            //rotSet[1].az = rotUdpEx.azUdp;
            //rotSet[1].el = rotUdpEx.elUdp;
            setPosition(1, rotUdpEx.azUdp, rotUdpEx.elUdp);
            ui->lineEdit_posAz_2->setText(QString::number(rotSet[1].az, 'f', 1) + " " + QString::number(rotSet[1].el, 'f', 1));
            //rot_set_position(my_rot2, rotSet[1].az, rotSet[1].el);
            break;
        case 2:
            //rotSet[2].az = rotUdpEx.azUdp;
            //rotSet[2].el = rotUdpEx.elUdp;
            setPosition(2, rotUdpEx.azUdp, rotUdpEx.elUdp);
            ui->lineEdit_posAz_3->setText(QString::number(rotSet[2].az, 'f', 1) + " " + QString::number(rotSet[2].el, 'f', 1));
            //rot_set_position(my_rot3, rotSet[2].az, rotSet[2].el);
            break;
        }
    }

    if (rotUdpEx.stopUdpFlag)
    {
        rotUdpEx.stopUdpFlag = false;
        emit ui->pushButton_stop->clicked(true);
    }

    if (rotUdpEx.parkUdpFlag)
    {
        rotUdpEx.parkUdpFlag = false;
        emit ui->pushButton_park->clicked(true);
    }

    //Tracking
    if (rotNumber == 0 && rotSet[0].trackFlag)
    {
        double tempAz=0, tempEl=0;

        if (rotSet[0].trackWSJTX) parseWSJTX(&tempAz, &tempEl);    //WSJT-X
        else if (rotSet[0].trackAirScout) parseAirScout(&tempAz, &tempEl);     //AirScout
        else if ((rotSet[0].trackPreviSat && rotUdpEx.previSatUdp) && (rotUdpEx.azUdpFlag || rotUdpEx.elUdpFlag))      //PreviSat
        {
            tempAz = rotUdpEx.azUdp;
            tempEl = rotUdpEx.elUdp;
            if (!rotUdpEx.satAOS && (tempEl >= rotSet[0].trackThreshold)) ui->statusbar->showMessage("Ready for tracking " + rotUdpEx.satName);
            else if (rotUdpEx.satAOS && (tempEl >= rotSet[0].trackThreshold)) ui->statusbar->showMessage("Tracking " + rotUdpEx.satName);
            else ui->statusbar->clearMessage();
            rotUdpEx.previSatUdp = false;
            rotUdpEx.azUdpFlag = false;
            rotUdpEx.elUdpFlag = false;
        }
        else
        {
            tempEl = -90;      //Data error flag
            ui->statusbar->clearMessage();
        }

        if (tempEl != -90)   //No error
        {
            if (((abs(tempAz - rotSet[0].az) > rotSet[0].trackTolerance) || (abs(tempEl - rotSet[0].el) > rotSet[0].trackTolerance)) && ((tempAz != rotSet[0].az) || (tempEl != rotSet[0].el)) && (tempEl >= rotSet[0].trackThreshold))
            {
                //rotSet[0].az = tempAz;
                //rotSet[0].el = tempEl;
                setPosition(0, tempAz, tempEl);
                ui->lineEdit_posAz->setText(QString::number(rotSet[0].az) + " " + QString::number(rotSet[0].el));
                //rot_set_position(my_rot, rotSet[0].az, rotSet[0].el);
            }
        }
    }

    if (rotNumber == 1 && rotSet[1].trackFlag)
    {
        double tempAz=0, tempEl=0;
        if (rotSet[1].trackWSJTX) parseWSJTX(&tempAz, &tempEl);
        else if (rotSet[1].trackAirScout) parseAirScout(&tempAz, &tempEl);
        else if ((rotSet[1].trackPreviSat && rotUdpEx.previSatUdp) && (rotUdpEx.azUdpFlag || rotUdpEx.elUdpFlag))
        {
            tempAz = rotUdpEx.azUdp;
            tempEl = rotUdpEx.elUdp;
            if (!rotUdpEx.satAOS && (tempEl >= rotSet[1].trackThreshold)) ui->statusbar->showMessage("Ready for tracking " + rotUdpEx.satName);
            else if (rotUdpEx.satAOS && (tempEl >= rotSet[1].trackThreshold)) ui->statusbar->showMessage("Tracking " + rotUdpEx.satName);
            else ui->statusbar->clearMessage();
            rotUdpEx.previSatUdp = false;
            rotUdpEx.azUdpFlag = false;
            rotUdpEx.elUdpFlag = false;
        }
        else
        {
            tempEl = -90;
            ui->statusbar->clearMessage();
        }

        if (tempEl != -90)   //No tracking dat error
        {
            if (((abs(tempAz - rotSet[1].az) > rotSet[1].trackTolerance) || (abs(tempEl - rotSet[1].el) > rotSet[1].trackTolerance)) && ((tempAz != rotSet[1].az) || (tempEl != rotSet[1].el)) && (tempEl >= rotSet[1].trackThreshold))
            {
                //rotSet[1].az = tempAz;
                //rotSet[1].el = tempEl;
                setPosition(1, tempAz, tempEl);
                ui->lineEdit_posAz_2->setText(QString::number(rotSet[1].az) + " " + QString::number(rotSet[1].el));
                //rot_set_position(my_rot2, rotSet[1].az, rotSet[1].el);
            }
        }
    }

    if (rotNumber == 2 && rotSet[2].trackFlag)
    {
        double tempAz=0, tempEl=0;
        if (rotSet[2].trackWSJTX) parseWSJTX(&tempAz, &tempEl);
        else if (rotSet[2].trackAirScout) parseAirScout(&tempAz, &tempEl);
        else if ((rotSet[2].trackPreviSat && rotUdpEx.previSatUdp) && (rotUdpEx.azUdpFlag || rotUdpEx.elUdpFlag))
        {
            tempAz = rotUdpEx.azUdp;
            tempEl = rotUdpEx.elUdp;
            if (!rotUdpEx.satAOS && (tempEl >= rotSet[2].trackThreshold)) ui->statusbar->showMessage("Ready for tracking " + rotUdpEx.satName);
            else if (rotUdpEx.satAOS && (tempEl >= rotSet[2].trackThreshold)) ui->statusbar->showMessage("Tracking " + rotUdpEx.satName);
            else ui->statusbar->clearMessage();
            rotUdpEx.previSatUdp = false;
            rotUdpEx.azUdpFlag = false;
            rotUdpEx.elUdpFlag = false;
        }
        else
        {
            tempEl = -90;
            ui->statusbar->clearMessage();
        }

        if (tempEl != -90)   //No tracking dat error
        {
            if (((abs(tempAz - rotSet[2].az) > rotSet[2].trackTolerance) || (abs(tempEl - rotSet[2].el) > rotSet[2].trackTolerance)) && ((tempAz != rotSet[2].az) || (tempEl != rotSet[2].el)) && (tempEl >= rotSet[2].trackThreshold))
            {
                //rotSet[2].az = tempAz;
                //rotSet[2].el = tempEl;
                setPosition(2, tempAz, tempEl);
                ui->lineEdit_posAz_3->setText(QString::number(rotSet[2].az) + " " + QString::number(rotSet[2].el));
                //rot_set_position(my_rot3, rotSet[2].az, rotSet[2].el);
            }
        }
    }
}

void MainWindow::presetGo(int presetNumber)
{
    switch (ui->tabWidget_rotator->currentIndex())
    {
    case 0:
        rotSet[0].az = rotCfg.presetAz[presetNumber];
        setPosition(0, rotSet[0].az, rotSet[0].el);
        ui->lineEdit_posAz->setText(QString::number(rotSet[0].az));
        //rot_set_position(my_rot, rotSet[0].az, rotSet[0].el);
        break;
    case 1:
        rotSet[1].az = rotCfg.presetAz[presetNumber];
        setPosition(1, rotSet[1].az, rotSet[1].el);
        ui->lineEdit_posAz_2->setText(QString::number(rotSet[1].az));
        //rot_set_position(my_rot2, rotSet[1].az, rotSet[1].el);
        break;
    case 2:
        rotSet[2].az = rotCfg.presetAz[presetNumber];
        setPosition(2, rotSet[2].az, rotSet[2].el);
        ui->lineEdit_posAz_3->setText(QString::number(rotSet[2].az));
        //rot_set_position(my_rot3, rotSet[2].az, rotSet[2].el);
        break;
    }
}

void MainWindow::presetInit()
{
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);

    configFile.beginReadArray("Preset");
    for (int i = 0; i < 9; i++)
    {
        configFile.setArrayIndex(i);
        rotCfg.presetLabel[i] = configFile.value("presetLabel", defaultPreset[i]).toString();
        rotCfg.presetAz[i] = configFile.value("presetAz", defaultPreset[i]).toInt();
    }
    configFile.endArray();

    ui->pushButton_p0->setText(rotCfg.presetLabel[0]);
    ui->pushButton_p1->setText(rotCfg.presetLabel[1]);
    ui->pushButton_p2->setText(rotCfg.presetLabel[2]);
    ui->pushButton_p3->setText(rotCfg.presetLabel[3]);
    ui->pushButton_p4->setText(rotCfg.presetLabel[4]);
    ui->pushButton_p5->setText(rotCfg.presetLabel[5]);
    ui->pushButton_p6->setText(rotCfg.presetLabel[6]);
    ui->pushButton_p7->setText(rotCfg.presetLabel[7]);
    ui->pushButton_p8->setText(rotCfg.presetLabel[8]);
}

void MainWindow::setPosition(int rot, float azim, float elev)
{
    switch (rot)
    {
    case 0:
        if (rotCom[0].connected)
        {
            if (my_rot->caps->rot_type == ROT_TYPE_ELEVATION)    //Elevation only rotator
            {
                rotSet[0].az = 0;
                rotSet[0].el = elev;
            }
            else //Azimuth or Az/El rotator
            {
                if (rotSet[0].overlap && rotGet[0].az>270 && azim>=0 && azim<=90 && my_rot->caps->max_az>360) rotSet[0].az = 360 + azim;
                else rotSet[0].az = azim;
                if (elev >= 0 && my_rot->caps->rot_type == ROT_TYPE_AZEL) rotSet[0].el = elev;
                else rotSet[0].el = 0;
            }
            rot_set_position(my_rot, rotSet[0].az, rotSet[0].el);
        }
        break;
    case 1:
        if (rotCom[1].connected)
        {
            if (my_rot2->caps->rot_type == ROT_TYPE_ELEVATION)
            {
                rotSet[1].az = 0;
                rotSet[1].el = elev;
            }
            else
            {
                if (rotSet[1].overlap && rotGet[1].az>270 && azim>=0 && azim<=90 && my_rot2->caps->max_az>360) rotSet[1].az = 360 + azim;
                else rotSet[1].az = azim;
                if (elev >= 0 && my_rot2->caps->rot_type == ROT_TYPE_AZEL) rotSet[1].el = elev;
                else rotSet[1].el = 0;
            }
            rot_set_position(my_rot2, rotSet[1].az, rotSet[1].el);
        }
        break;
    case 2:
        if (rotCom[2].connected)
        {
            if (my_rot3->caps->rot_type == ROT_TYPE_ELEVATION)
            {
                rotSet[2].az = 0;
                rotSet[2].el = elev;
            }
            else
            {
                if (rotSet[2].overlap && rotGet[2].az>270 && azim>=0 && azim<=90 && my_rot3->caps->max_az>360) rotSet[2].az = 360 + azim;
                else rotSet[2].az = azim;
                if (elev >= 0 && my_rot3->caps->rot_type == ROT_TYPE_AZEL) rotSet[2].el = elev;
                else rotSet[2].el = 0;
            }
            rot_set_position(my_rot3, rotSet[2].az, rotSet[2].el);
        }
        break;
    }

    return;
}

void MainWindow::parseWSJTX(double *azim, double *elev)
{
    QFile azelDatWSJTX(rotCfg.pathTrackWSJTX + "/azel.dat");
    if (!azelDatWSJTX.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        *elev = -90; //Used for error
        return;
    }

    while (!azelDatWSJTX.atEnd())
    {
        QString datWSJTX = azelDatWSJTX.readLine();
        //QRegularExpression azelMoonDat("Moon"); //Find Moon line
        //QRegularExpressionMatch azelMoonDatMatch = azelMoonDat.match(datWSJTX);

        //if (azelMoonDatMatch.hasMatch())
        //{
            QRegularExpression azelDat(",?\\s*(\\d+\\.?\\d*),?\\s*(\\d+\\.?\\d*),Moon");   //Match x.x,x.x,Moon
            QRegularExpressionMatch azelDatMatch = azelDat.match(datWSJTX);

            if (azelDatMatch.hasMatch())
            {
                *azim = azelDatMatch.captured(1).toDouble();
                *elev = azelDatMatch.captured(2).toDouble();

                break;
            }
            else *elev = -90;    //Data error flag
        //}
    }

    azelDatWSJTX.close();
    return;
}

void MainWindow::parseAirScout(double *azim, double *elev)
{
    QFile azelDatAirScout(rotCfg.pathTrackAirScout + "/azel.dat");
    if (!azelDatAirScout.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        *elev = -90; //Used for error
        return;
    }
    QString datAirScout = azelDatAirScout.readLine(0);

    QRegularExpression azelDat("^(\\d+\\.?\\d*),(\\d+\\.?\\d*)");   //Match x.x,x.x
    QRegularExpressionMatch azelDatMatch = azelDat.match(datAirScout);

    if (azelDatMatch.hasMatch())
    {
        *azim = azelDatMatch.captured(1).toDouble();
        *elev = azelDatMatch.captured(2).toDouble();
    }
    else *elev = -90;    //Data error flag

    azelDatAirScout.close();
    return;
}

bool MainWindow::azElInput(QString value, bool lPath, double *azim, double *elev)
{
    QRegularExpression azCmdDeg("^-?\\d\\d?\\d?[.]?\\d?");    //Match 0, 00, 000 with or w/o "-" with or w/o decimal
    QRegularExpressionMatch azCmdDegMatch = azCmdDeg.match(value);
    QRegularExpression azElCmdDeg("^(-?\\d\\d?\\d?[.]?\\d?) (-?\\d\\d?[.]?\\d?)");
    QRegularExpressionMatch azElCmdDegMatch = azElCmdDeg.match(value);
    QRegularExpression azCmdLoc("^[a-rA-R][a-rA-R]\\d{2}([a-xA-X][a-xA-X])?(\\d{2})?"); //Match AA00, AA00AA, AA00AA00
    QRegularExpressionMatch azCmdLocMatch = azCmdLoc.match(value);

    double dist = 0;
    double tempAz, tempEl;

    *elev = -1; //Used for no elevation input

    if (azElCmdDegMatch.hasMatch()) //Az && El
    {
        tempAz = azElCmdDegMatch.captured(1).toDouble();
        if (lPath) *azim = azimuth_long_path(tempAz);
        else *azim = tempAz;
        tempEl = azElCmdDegMatch.captured(2).toDouble();
        *elev = tempEl;
        ui->statusbar->clearMessage();
        return true;
    }
    else if (azCmdDegMatch.hasMatch())  //Az or El
    {
        tempAz = azCmdDegMatch.captured(0).toDouble();
        if (lPath) *azim = azimuth_long_path(tempAz);
        else *azim = tempAz;
        ui->statusbar->clearMessage();
        return true;
    }
    else if (azCmdLocMatch.hasMatch() && rotCfg.qthLocator != "")   //Locator
    {
        if (lPath)  //Long Path
        {
            if (MainWindow::bearingAngleLP(rotCfg.qthLocator.toLatin1(), azCmdLocMatch.captured(0).toLatin1(), &tempAz, &dist))
            {
                *azim = tempAz;
                ui->statusbar->showMessage("Bearing LP "+QString::number(*azim,'f',1)+" deg, Distance "+QString::number(dist,'f',0)+" km");
                return true;
            }
            else return false;
        }
        //Short Path
        else if (MainWindow::bearingAngle(rotCfg.qthLocator.toLatin1(), azCmdLocMatch.captured(0).toLatin1(), &tempAz, &dist))
        {
            *azim = tempAz;
            ui->statusbar->showMessage("Bearing SP "+QString::number(*azim,'f',1)+" deg, Distance "+QString::number(dist,'f',0)+" km");
            return true;
        }
        else return false;
    }
    else
    {
        *azim = 0;
        return false;
    }
}

bool MainWindow::bearingAngle(const char *locator1, const char *locator2, double *azim, double *dist)
{
    double lon1, lon2;
    double lat1, lat2;

    if (locator2longlat(&lon1, &lat1, locator1) == RIG_OK && locator2longlat(&lon2, &lat2, locator2) == RIG_OK)
    {
        qrb(lon1, lat1, lon2, lat2, dist, azim);
        return true;
    }
    else return false;
}

bool MainWindow::bearingAngleLP(const char *locator1, const char *locator2, double *azim, double *dist)
{
    double lon1, lon2;
    double lat1, lat2;
    double tempAz, tempDist;

    if (locator2longlat(&lon1, &lat1, locator1) == RIG_OK && locator2longlat(&lon2, &lat2, locator2) == RIG_OK)
    {
        qrb(lon1, lat1, lon2, lat2, &tempDist, &tempAz);
        *azim = azimuth_long_path(tempAz);
        *dist = distance_long_path(tempDist);
        return true;
    }
    else return false;
}

//* Buttons
void MainWindow::on_pushButton_connect_toggled(bool checked)
{
    QString connectMsg;

    if (checked)
    {
       my_rot = rotDaemon->rotConnect(&rotCom[0]);   //Open Rotator connection
       if (!my_rot) rotCom[0].connected = 0;
       else rotCom[0].connected = 1;

       if (rotSet[1].enable)
       {
           my_rot2 = rotDaemon->rotConnect(&rotCom[1]);
           if (!my_rot2) rotCom[1].connected = 0;
           else rotCom[1].connected = 1;
       }

       if (rotSet[2].enable)
       {
           my_rot3 = rotDaemon->rotConnect(&rotCom[2]);
           if (!my_rot3) rotCom[2].connected = 0;
           else rotCom[2].connected = 1;
       }

       if (!rotCom[0].connected && (rotSet[1].enable && !rotCom[1].connected) && (rotSet[2].enable && !rotCom[2].connected))   //Connection error
       {
           connectMsg = "Connection error!";
           ui->pushButton_connect->setChecked(false);  //Uncheck the button
       }
       else    //Rotator connected
       {
           timer->start(rotCfg.rotRefresh*1000);
           guiInit();

           connectMsg = "Connected";
           if (rotCom[0].connected) connectMsg = connectMsg + " " + rotSet[0].nameLabel;
           if (rotCom[1].connected) connectMsg = connectMsg + " " + rotSet[1].nameLabel;
           if (rotCom[2].connected) connectMsg = connectMsg + " " + rotSet[2].nameLabel;
       }
    }
    else   //Button unchecked
    {
        timer->stop();

        if (rotCom[0].connected)
        {
            rot_close(my_rot);  //Close the communication to the rotator
            rotCom[0].connected = 0;
        }

        if (rotSet[1].enable && rotCom[1].connected)
        {
            rot_close(my_rot2);
            rotCom[1].connected = 0;
        }

        if (rotSet[2].enable && rotCom[2].connected)
        {
            rot_close(my_rot3);
            rotCom[2].connected = 0;
        }
        connectMsg = "Disconnected";
    }

    ui->statusbar->showMessage(connectMsg);
}

void MainWindow::on_pushButton_stop_clicked()
{
    if (rotCom[0].connected)
    {
        rotSet[0].trackFlag = false; //stop tracking (if any)
        ui->toolButton_track->setChecked(false);
        rot_stop(my_rot);   //send stop command
        rotSet[0].az = rotGet[0].az;  //retrieve last position
        rotSet[0].el = rotGet[0].el;
    }

    if (rotCom[1].connected)
    {
        rotSet[1].trackFlag = false;
        ui->toolButton_track_2->setChecked(false);
        rot_stop(my_rot2);
        rotSet[1].az = rotGet[1].az;
        rotSet[1].el = rotGet[1].el;
    }

    if (rotCom[2].connected)
    {
        rotSet[2].trackFlag = false;
        ui->toolButton_track_3->setChecked(false);
        rot_stop(my_rot3);
        rotSet[2].az = rotGet[2].az;
        rotSet[2].el = rotGet[2].el;
    }

    ui->statusbar->showMessage("Stop all");
}


//Rotor 1
void MainWindow::on_pushButton_go_clicked()
{
   double tempAz, tempEl;
   if (MainWindow::azElInput(ui->lineEdit_posAz->text(), rotSet[0].lPathFlag, &tempAz, &tempEl))
   {
       //rot_set_position(my_rot, rotSet[0].az, rotSet[0].el);
       setPosition(0, tempAz, tempEl);
       ui->lineEdit_posAz->setText(QString::number(rotSet[0].az,'f',1));
   }
}

void MainWindow::on_toolButton_minus_clicked()
{
    rotSet[0].az = rotSet[0].az - rotCfg.incrementAz;
    ui->lineEdit_posAz->setText(QString::number(rotSet[0].az,'f',1));
}


void MainWindow::on_toolButton_plus_clicked()
{
    rotSet[0].az = rotSet[0].az + rotCfg.incrementAz;
    ui->lineEdit_posAz->setText(QString::number(rotSet[0].az,'f',1));
}

void MainWindow::on_toolButton_pathSL_toggled(bool checked)
{
    if (checked)
    {
        ui->toolButton_pathSL->setText("LP");
        rotSet[0].lPathFlag = true;
    }
    else
    {
        ui->toolButton_pathSL->setText("SP");
        rotSet[0].lPathFlag = false;
    }
    emit ui->pushButton_go->clicked(true);
}

void MainWindow::on_toolButton_track_toggled(bool checked)
{
    if (checked)
    {
        if (rotSet[0].trackPreviSat)
        {
            ui->toolButton_track->setText("SAT");
            ui->statusbar->showMessage("Tracking PreviSat " + rotSet[0].nameLabel);
        }
        else if (rotSet[0].trackWSJTX)
        {
            ui->toolButton_track->setText("WSJ");
            ui->statusbar->showMessage("Tracking WSJT-X (Moon) " + rotSet[0].nameLabel);
        }
        else if (rotSet[0].trackAirScout)
        {
            ui->toolButton_track->setText("AP");
            ui->statusbar->showMessage("Tracking AirScout " + rotSet[0].nameLabel);
        }
        else
        {
            ui->toolButton_track->setChecked(false);
            return;
        }
        rotSet[0].trackFlag = true;
    }
    else
    {
        ui->toolButton_track->setText("TRK");
        ui->statusbar->showMessage("Tracking off " + rotSet[0].nameLabel);
        rotSet[0].trackFlag = false;
    }
}

//Rotor 2
void MainWindow::on_pushButton_go_2_clicked()
{
    double tempAz, tempEl;
    if (MainWindow::azElInput(ui->lineEdit_posAz_2->text(), rotSet[1].lPathFlag, &tempAz, &tempEl))
    {
        //rot_set_position(my_rot2, rotSet[1].az, rotSet[1].el);
        setPosition(1, tempAz, tempEl);
        ui->lineEdit_posAz_2->setText(QString::number(rotSet[1].az));
    }
}

void MainWindow::on_toolButton_minus_2_clicked()
{
    rotSet[1].az = rotSet[1].az - rotCfg.incrementAz;
    ui->lineEdit_posAz_2->setText(QString::number(rotSet[1].az));
}

void MainWindow::on_toolButton_plus_2_clicked()
{
    rotSet[1].az = rotSet[1].az + rotCfg.incrementAz;
    ui->lineEdit_posAz_2->setText(QString::number(rotSet[1].az));
}

void MainWindow::on_toolButton_pathSL_2_toggled(bool checked)
{
    if (checked)
    {
        ui->toolButton_pathSL_2->setText("LP");
        rotSet[1].lPathFlag = true;
    }
    else
    {
        ui->toolButton_pathSL_2->setText("SP");
        rotSet[1].lPathFlag = false;
    }
    emit ui->pushButton_go_2->clicked(true);
}

void MainWindow::on_toolButton_track_2_toggled(bool checked)
{
    if (checked)
    {
        if (rotSet[1].trackPreviSat)
        {
            ui->toolButton_track_2->setText("SAT");
            ui->statusbar->showMessage("Tracking PreviSat " + rotSet[1].nameLabel);
        }
        else if (rotSet[1].trackWSJTX)
        {
            ui->toolButton_track_2->setText("WSJ");
            ui->statusbar->showMessage("Tracking WSJT-X (Moon) " + rotSet[1].nameLabel);
        }
        else if (rotSet[1].trackAirScout)
        {
            ui->toolButton_track_2->setText("AP");
            ui->statusbar->showMessage("Tracking AirScout " + rotSet[1].nameLabel);
        }
        else
        {
            ui->toolButton_track_2->setChecked(false);
            return;
        }
        rotSet[1].trackFlag = true;
    }
    else
    {
        ui->toolButton_track_2->setText("TRK");
        ui->statusbar->showMessage("Tracking off " + rotSet[1].nameLabel);
        rotSet[1].trackFlag = false;
    }
}

//Rotor 3
void MainWindow::on_pushButton_go_3_clicked()
{
    double tempAz, tempEl;
    if (MainWindow::azElInput(ui->lineEdit_posAz_3->text(), rotSet[2].lPathFlag, &tempAz, &tempEl))
    {
        //rot_set_position(my_rot3, rotSet[2].az, rotSet[2].el);
        setPosition(2, tempAz, tempEl);
        ui->lineEdit_posAz_3->setText(QString::number(rotSet[2].az));
    }
}

void MainWindow::on_toolButton_minus_3_clicked()
{
    rotSet[2].az = rotSet[2].az - rotCfg.incrementAz;
    ui->lineEdit_posAz_3->setText(QString::number(rotSet[2].az));
}

void MainWindow::on_toolButton_plus_3_clicked()
{
    rotSet[2].az = rotSet[2].az + rotCfg.incrementAz;
    ui->lineEdit_posAz_3->setText(QString::number(rotSet[2].az));
}

void MainWindow::on_toolButton_pathSL_3_toggled(bool checked)
{
    if (checked)
    {
        ui->toolButton_pathSL_3->setText("LP");
        rotSet[2].lPathFlag = true;
    }
    else
    {
        ui->toolButton_pathSL_3->setText("SP");
        rotSet[2].lPathFlag = false;
    }
    emit ui->pushButton_go_3->clicked(true);
}

void MainWindow::on_toolButton_track_3_toggled(bool checked)
{
    if (checked)
    {
        if (rotSet[2].trackPreviSat)
        {
            ui->toolButton_track_3->setText("SAT");
            ui->statusbar->showMessage("Tracking PreviSat " + rotSet[2].nameLabel);
        }
        else if (rotSet[2].trackWSJTX)
        {
            ui->toolButton_track_3->setText("WSJ");
            ui->statusbar->showMessage("Tracking WSJT-X (Moon) " + rotSet[2].nameLabel);
        }
        else if (rotSet[2].trackAirScout)
        {
            ui->toolButton_track_3->setText("AP");
            ui->statusbar->showMessage("Tracking AirScout " + rotSet[2].nameLabel);
        }
        else
        {
            ui->toolButton_track_3->setChecked(false);
            return;
        }
        rotSet[2].trackFlag = true;
    }
    else
    {
        ui->toolButton_track_3->setText("TRK");
        ui->statusbar->showMessage("Tracking off " + rotSet[2].nameLabel);
        rotSet[2].trackFlag = false;
    }
}

//Park
void MainWindow::on_pushButton_park_clicked()
{
    switch (ui->tabWidget_rotator->currentIndex())
    {
    case 0:
        if (my_rot->caps->park) rot_park(my_rot);
        else
        {
            rotSet[0].az = rotSet[0].azPark;
            rotSet[0].el = rotSet[0].elPark;
            ui->lineEdit_posAz->setText(QString::number(rotSet[0].az));
            rot_set_position(my_rot, rotSet[0].az, rotSet[0].el);
        }
        break;

    case 1:
        if (my_rot2->caps->park) rot_park(my_rot2);
        else
        {
            rotSet[1].az = rotSet[1].azPark;
            rotSet[1].el = rotSet[1].elPark;
            ui->lineEdit_posAz_2->setText(QString::number(rotSet[1].az));
            rot_set_position(my_rot2, rotSet[1].az, rotSet[1].el);
        }
        break;

    case 2:
        if (my_rot3->caps->park) rot_park(my_rot3);
        else
        {
            rotSet[2].az = rotSet[2].azPark;
            rotSet[2].el = rotSet[2].elPark;
            ui->lineEdit_posAz_3->setText(QString::number(rotSet[2].az));
            rot_set_position(my_rot3, rotSet[2].az, rotSet[2].el);
        }
        break;
    }
}

//Presets
void MainWindow::on_pushButton_p0_clicked()
{
    presetGo(0);
}

void MainWindow::on_pushButton_p1_clicked()
{
    presetGo(1);
}

void MainWindow::on_pushButton_p2_clicked()
{
    presetGo(2);
}

void MainWindow::on_pushButton_p3_clicked()
{
    presetGo(3);
}

void MainWindow::on_pushButton_p4_clicked()
{
    presetGo(4);
}


void MainWindow::on_pushButton_p5_clicked()
{
    presetGo(5);
}

void MainWindow::on_pushButton_p6_clicked()
{
    presetGo(6);
}

void MainWindow::on_pushButton_p7_clicked()
{
    presetGo(7);
}

void MainWindow::on_pushButton_p8_clicked()
{
    presetGo(8);
}


//* Menu
void MainWindow::on_actionRotator_triggered()
{
    DialogRotator config;
    //config.setModal(true);
    config.exec();
}

void MainWindow::on_actionSetup_triggered()
{
    DialogSetup setup;
    //setup.setModal(true);
    setup.exec();
}

void MainWindow::on_actionPresets_triggered()
{
    DialogPreset preset;
    connect(&preset, &DialogPreset::configDone, this, &MainWindow::presetInit);
    //preset.setModal(true);
    preset.exec();
}

void MainWindow::on_actionCatRotator_homepage_triggered()
{
    QUrl homepage("https://www.pianetaradio.it/blog/catrotator/");
    QDesktopServices::openUrl(homepage);
}

void MainWindow::on_actionAbout_CatRotator_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About");
    msgBox.setTextFormat(Qt::RichText);
    QString version = QString::number(VERSION_MAJ)+"."+QString::number(VERSION_MIN)+"."+QString::number(VERSION_MIC);
    msgBox.setText("<b>CatRotator</b> <i>Rotator control software</i><br/>version "+version+" "+RELEASE_DATE);
    msgBox.setInformativeText("Copyright (C) 2022 Gianfranco Sordetti IZ8EWD<br/>"
                              "<a href='https://www.pianetaradio.it'>www.pianetaradio.it</a></p>"
                              "<p>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.<br/>"
                              "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br/>"
                              "You should have received a copy of the GNU General Public License along with this program.  If not, see <a href='http://www.gnu.org/licenses/'>www.gnu.org/licenses</a>.");
    msgBox.setIcon(QMessageBox::NoIcon);
    msgBox.setStandardButtons(QMessageBox::Ok);

    QPixmap icon("catrotator.png");
    msgBox.setIconPixmap(icon);

    msgBox.exec();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionAbout_Hamlib_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About Hamlib");
    msgBox.setText(rig_version());
    msgBox.setInformativeText(rig_copyright());
    //msgBox.setDetailedText(rig_license());
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

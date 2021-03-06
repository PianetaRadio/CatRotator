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

#include <rotator.h>    //Hamlib

ROT *my_rot;
ROT *my_rot2;
ROT *my_rot3;

extern rotatorConnect rotCom;
extern rotatorSettings rotGet;
extern rotatorSettings rotSet;

extern rotatorConnect rotCom2;
extern rotatorSettings rotGet2;
extern rotatorSettings rotSet2;

extern rotatorConnect rotCom3;
extern rotatorSettings rotGet3;
extern rotatorSettings rotSet3;

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

    rotCom.rotModel = configFile.value("Rotator1/rotModel", 0).toInt();
    rotCom.rotPort = configFile.value("Rotator1/rotPort").toString();
    rotCom.serialSpeed = configFile.value("Rotator1/serialSpeed", 9600).toInt();
    rotCom.netRotctl = configFile.value("Rotator1/netRotctl", false).toBool();
    if (rotCom.rotModel) rotSet.enable = true;
    rotSet.nameLabel = configFile.value("Rotator1/nameLabel", "Rotator 1").toString();
    rotSet.azPark = configFile.value("Rotator1/azPark", 0).toInt();
    rotSet.elPark = configFile.value("Rotator1/elPark", 0).toInt();

    rotCom2.rotModel = configFile.value("Rotator2/rotModel", 0).toInt();
    rotCom2.rotPort = configFile.value("Rotator2/rotPort").toString();
    rotCom2.serialSpeed = configFile.value("Rotator2/serialSpeed", 9600).toInt();
    rotCom2.netRotctl = configFile.value("Rotator2/netRotctl", false).toBool();
    if (rotCom2.rotModel) rotSet2.enable = true;
    rotSet2.nameLabel = configFile.value("Rotator2/nameLabel", "Rotator 2").toString();
    rotSet2.azPark = configFile.value("Rotator2/azPark", 0).toInt();
    rotSet2.elPark = configFile.value("Rotator2/elPark", 0).toInt();

    rotCom3.rotModel = configFile.value("Rotator3/rotModel", 0).toInt();
    rotCom3.rotPort = configFile.value("Rotator3/rotPort").toString();
    rotCom3.serialSpeed = configFile.value("Rotator3/serialSpeed", 9600).toInt();
    rotCom3.netRotctl = configFile.value("Rotator3/netRotctl", false).toBool();
    if (rotCom3.rotModel) rotSet3.enable = true;
    rotSet3.nameLabel = configFile.value("Rotator3/nameLabel", "Rotator 3").toString();
    rotSet3.azPark = configFile.value("Rotator3/azPark", 0).toInt();
    rotSet3.elPark = configFile.value("Rotator3/elPark", 0).toInt();

    rotCfg.rotRefresh = configFile.value("rotRefresh", 1).toInt();
    rotCfg.qthLocator = configFile.value("qthLocator", "").toString();
    rotCfg.udp = configFile.value("udp", false).toBool();
    rotCfg.udpAddress = configFile.value("udpAddress", "127.0.0.1").toString();
    rotCfg.udpPort = configFile.value("udpPort", 12000).toUInt();   //should be toUShort()

    //Presets
    //std::copy(defaultPreset, defaultPreset+9, rotCfg.preset);
    MainWindow::presetInit();

    //Window settings
    restoreGeometry(configFile.value("WindowSettings/geometry").toByteArray());
    restoreState(configFile.value("WindowSettings/state").toByteArray());
}

MainWindow::~MainWindow()
{
    workerThread.quit();
    workerThread.wait();

    if (rotCom.connected)
    {
        timer->stop();

        rotCom.connected = 0;
        rot_close(my_rot);  //Close the communication to the rotator
    }
    rot_cleanup(my_rot);    //Release rot handle and free associated memory

    //* Save window settings
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    configFile.setValue("WindowSettings/geometry", saveGeometry());
    configFile.setValue("WindowSettings/state", saveState());

    delete ui;
}

//* Get values
void MainWindow::rotUpdate()
{
    if (rotCom.connected) rotDaemon->rotUpdate(my_rot, &rotGet);
    if (rotSet2.enable && rotCom2.connected) rotDaemon->rotUpdate(my_rot2, &rotGet2);
    if (rotSet3.enable && rotCom3.connected) rotDaemon->rotUpdate(my_rot3, &rotGet3);
}

//* RotDaemon handle results
void MainWindow::on_rotDaemonResultReady()
{
    guiUpdate();
}

void MainWindow::guiInit()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ui->tabWidget_rotator->removeTab(3);
#endif

    if (rotSet3.enable)
    {
        ui->tabWidget_rotator->setTabText(2, rotSet3.nameLabel);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ui->tabWidget_rotator->setTabVisible(2, true);
#endif
        if (rotCom3.connected)
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

    if (rotSet2.enable)
    {
        ui->tabWidget_rotator->setTabText(1, rotSet2.nameLabel);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ui->tabWidget_rotator->setTabVisible(1, true);
#endif
        if (rotCom2.connected)
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

    ui->tabWidget_rotator->setTabText(0, rotSet.nameLabel);
    if (rotCom.connected)
    {
        ui->tabWidget_rotator->setTabEnabled(0, true);
        if (my_rot->caps->rot_type == ROT_TYPE_AZIMUTH) ui->lcdNumber_posEl->setVisible(false);
        if (my_rot->caps->rot_type == ROT_TYPE_ELEVATION) ui->toolButton_pathSL->setVisible(false);
        //ui->spinBox_posAz->setMaximum(my_rot->caps->max_az);
        //ui->spinBox_posAz->setMinimum(my_rot->caps->min_az);
    }
    else ui->tabWidget_rotator->setTabEnabled(0, false);
}

void MainWindow::guiUpdate()
{
    //Update current position
    ui->lcdNumber_posAz->display(rotGet.az);
    ui->lcdNumber_posEl->display(rotGet.el);

    if (rotSet2.enable)
    {
        ui->lcdNumber_posAz_2->display(rotGet2.az);
        ui->lcdNumber_posEl_2->display(rotGet2.el);
    }

    if (rotSet3.enable)
    {
        ui->lcdNumber_posAz_3->display(rotGet3.az);
        ui->lcdNumber_posEl_3->display(rotGet3.el);
    }

    //Parse UDP command
    if (rotUdpEx.azUdpFlag || rotUdpEx.elUdpFlag)
    {
        rotUdpEx.azUdpFlag = false;
        rotUdpEx.elUdpFlag = false;

        switch (ui->tabWidget_rotator->currentIndex())
        {
        case 0:
            rotSet.az = rotUdpEx.azUdp;
            rotSet.el = rotUdpEx.elUdp;
            ui->lineEdit_posAz->setText(QString::number(rotSet.az));
            rot_set_position(my_rot, rotSet.az, rotSet.el);
            break;
        case 1:
            rotSet2.az = rotUdpEx.azUdp;
            rotSet2.el = rotUdpEx.elUdp;
            ui->lineEdit_posAz_2->setText(QString::number(rotSet2.az));
            rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
            break;
        case 2:
            rotSet3.az = rotUdpEx.azUdp;
            rotSet3.el = rotUdpEx.elUdp;
            ui->lineEdit_posAz_3->setText(QString::number(rotSet3.az));
            rot_set_position(my_rot3, rotSet3.az, rotSet3.el);
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
}

void MainWindow::presetGo(int presetNumber)
{
    switch (ui->tabWidget_rotator->currentIndex())
    {
    case 0:
        rotSet.az = rotCfg.presetAz[presetNumber];
        ui->lineEdit_posAz->setText(QString::number(rotSet.az));
        rot_set_position(my_rot, rotSet.az, rotSet.el);
        break;
    case 1:
        rotSet2.az = rotCfg.presetAz[presetNumber];
        ui->lineEdit_posAz_2->setText(QString::number(rotSet2.az));
        rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
        break;
    case 2:
        rotSet3.az = rotCfg.presetAz[presetNumber];
        ui->lineEdit_posAz_3->setText(QString::number(rotSet3.az));
        rot_set_position(my_rot3, rotSet3.az, rotSet3.el);
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

bool MainWindow::azElInput(QString value, bool lPath, double *azim, double *elev)
{
    QRegularExpression azCmdDeg("^-?\\d\\d?\\d?");    //Match 0, 00, 000 with or w/o -
    QRegularExpressionMatch azCmdDegMatch = azCmdDeg.match(value);
    QRegularExpression azElCmdDeg("^(-?\\d\\d?\\d?) (-?\\d\\d?\\d?)");
    QRegularExpressionMatch azElCmdDegMatch = azElCmdDeg.match(value);
    QRegularExpression azCmdLoc("^[a-rA-R][a-rA-R]\\d{2}([a-xA-X][a-xA-X])?(\\d{2})?"); //Match AA00, AA00AA, AA00AA00
    QRegularExpressionMatch azCmdLocMatch = azCmdLoc.match(value);

    double dist = 0;
    double tempAz, tempEl;

    *elev = -1; //Used for no elevation input

    if (azElCmdDegMatch.hasMatch())
    {
        tempAz = azElCmdDegMatch.captured(1).toDouble();
        if (lPath) *azim = azimuth_long_path(tempAz);
        else *azim = tempAz;
        tempEl = azElCmdDegMatch.captured(2).toDouble();
        *elev = tempEl;
        ui->statusbar->clearMessage();
        return true;
    }
    else if (azCmdDegMatch.hasMatch())
    {
        tempAz = azCmdDegMatch.captured(0).toDouble();
        if (lPath) *azim = azimuth_long_path(tempAz);
        else *azim = tempAz;
        ui->statusbar->clearMessage();
        return true;
    }
    else if (azCmdLocMatch.hasMatch() && rotCfg.qthLocator != "")
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
       my_rot = rotDaemon->rotConnect(&rotCom);   //Open Rotator connection
       if (!my_rot) rotCom.connected = 0;
       else rotCom.connected = 1;

       if (rotSet2.enable)
       {
           my_rot2 = rotDaemon->rotConnect(&rotCom2);
           if (!my_rot2) rotCom2.connected = 0;
           else rotCom2.connected = 1;
       }

       if (rotSet3.enable)
       {
           my_rot3 = rotDaemon->rotConnect(&rotCom3);
           if (!my_rot3) rotCom3.connected = 0;
           else rotCom3.connected = 1;
       }

       if (!rotCom.connected && (rotSet2.enable && !rotCom2.connected) && (rotSet3.enable && !rotCom3.connected))   //Connection error
       {
           connectMsg = "Connection error!";
           ui->pushButton_connect->setChecked(false);  //Uncheck the button
       }
       else    //Rotator connected
       {
           timer->start(rotCfg.rotRefresh*1000);
           guiInit();

           connectMsg = "Connected";
           if (rotCom.connected) connectMsg = connectMsg + " " + rotSet.nameLabel;
           if (rotCom2.connected) connectMsg = connectMsg + " " + rotSet2.nameLabel;
           if (rotCom3.connected) connectMsg = connectMsg + " " + rotSet3.nameLabel;
       }
    }
    else   //Button unchecked
    {
        timer->stop();

        if (rotCom.connected)
        {
            rot_close(my_rot);  //Close the communication to the rotator
            rotCom.connected = 0;
        }

        if (rotSet2.enable && rotCom2.connected)
        {
            rot_close(my_rot2);
            rotCom2.connected = 0;
        }

        if (rotSet3.enable && rotCom3.connected)
        {
            rot_close(my_rot3);
            rotCom3.connected = 0;
        }
        connectMsg = "Disconnected";
    }

    ui->statusbar->showMessage(connectMsg);
}

void MainWindow::on_pushButton_stop_clicked()
{
    rot_stop(my_rot);
    if (rotSet2.enable) rot_stop(my_rot2);
    if (rotSet3.enable) rot_stop(my_rot3);
}

void MainWindow::on_pushButton_go_clicked()
{
   double tempAz, tempEl;
   if (MainWindow::azElInput(ui->lineEdit_posAz->text(), rotSet.lPathFlag, &tempAz, &tempEl))
   {
       if (my_rot->caps->rot_type == ROT_TYPE_ELEVATION)    //Elevation only rotator
       {
           rotSet.az = 0;
           rotSet.el = tempAz;
       }
       else //Azimuth or Az/El rotator
       {
           rotSet.az = tempAz;
           if (tempEl >= 0) rotSet.el = tempEl;
       }
       rot_set_position(my_rot, rotSet.az, rotSet.el);
   }
}

void MainWindow::on_toolButton_pathSL_toggled(bool checked)
{
    if (checked)
    {
        ui->toolButton_pathSL->setText("LP");
        rotSet.lPathFlag = true;
    }
    else
    {
        ui->toolButton_pathSL->setText("SP");
        rotSet.lPathFlag = false;
    }
    emit ui->pushButton_go->clicked(true);
}

void MainWindow::on_pushButton_go_2_clicked()
{
    double tempAz, tempEl;
    if (MainWindow::azElInput(ui->lineEdit_posAz_2->text(), rotSet2.lPathFlag, &tempAz, &tempEl))
    {
        if (my_rot2->caps->rot_type == ROT_TYPE_ELEVATION)
        {
            rotSet2.az = 0;
            rotSet2.el = tempAz;
        }
        else
        {
            rotSet2.az = tempAz;
            if (tempEl >= 0) rotSet2.el = tempEl;
        }
        rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
    }
}

void MainWindow::on_toolButton_pathSL_2_toggled(bool checked)
{
    if (checked)
    {
        ui->toolButton_pathSL_2->setText("LP");
        rotSet2.lPathFlag = true;
    }
    else
    {
        ui->toolButton_pathSL_2->setText("SP");
        rotSet2.lPathFlag = false;
    }
    emit ui->pushButton_go_2->clicked(true);
}

void MainWindow::on_pushButton_go_3_clicked()
{
    double tempAz, tempEl;
    if (MainWindow::azElInput(ui->lineEdit_posAz_3->text(), rotSet3.lPathFlag, &tempAz, &tempEl))
    {
        if (my_rot3->caps->rot_type == ROT_TYPE_ELEVATION)
        {
            rotSet3.az = 0;
            rotSet3.el = tempAz;
        }
        else
        {
            rotSet3.az = tempAz;
            if (tempEl >= 0) rotSet3.el = tempEl;
        }
        rot_set_position(my_rot3, rotSet3.az, rotSet3.el);
    }
}

void MainWindow::on_toolButton_pathSL_3_toggled(bool checked)
{
    if (checked)
    {
        ui->toolButton_pathSL_3->setText("LP");
        rotSet3.lPathFlag = true;
    }
    else
    {
        ui->toolButton_pathSL_3->setText("SP");
        rotSet3.lPathFlag = false;
    }
    emit ui->pushButton_go_3->clicked(true);
}

void MainWindow::on_pushButton_park_clicked()
{
    switch (ui->tabWidget_rotator->currentIndex())
    {
    case 0:
        if (my_rot->caps->park) rot_park(my_rot);
        else
        {
            rotSet.az = rotSet.azPark;
            rotSet.el = rotSet.elPark;
            ui->lineEdit_posAz->setText(QString::number(rotSet.az));
            rot_set_position(my_rot, rotSet.az, rotSet.el);
        }
        break;

    case 1:
        if (my_rot2->caps->park) rot_park(my_rot2);
        else
        {
            rotSet2.az = rotSet2.azPark;
            rotSet2.el = rotSet2.elPark;
            ui->lineEdit_posAz_2->setText(QString::number(rotSet2.az));
            rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
        }
        break;

    case 2:
        if (my_rot3->caps->park) rot_park(my_rot3);
        else
        {
            rotSet3.az = rotSet3.azPark;
            rotSet3.el = rotSet3.elPark;
            ui->lineEdit_posAz_3->setText(QString::number(rotSet3.az));
            rot_set_position(my_rot3, rotSet3.az, rotSet3.el);
        }
        break;
    }
}

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
    config.setModal(true);
    config.exec();
}

void MainWindow::on_actionSetup_triggered()
{
    DialogSetup setup;
    setup.setModal(true);
    setup.exec();
}

void MainWindow::on_actionPresets_triggered()
{
    DialogPreset preset;
    connect(&preset, &DialogPreset::configDone, this, &MainWindow::presetInit);
    preset.setModal(true);
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

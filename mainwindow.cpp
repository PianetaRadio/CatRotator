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
#include "rotatordata.h"
#include "rotdaemon.h"

#include <QMessageBox>
#include <QThread>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>

#include <rotator.h>    //Hamlib

ROT *my_rot;
ROT *my_rot2;

extern rotatorConnect rotCom;
extern rotatorSettings rotGet;
extern rotatorSettings rotSet;

extern rotatorConnect rotCom2;
extern rotatorSettings rotGet2;
extern rotatorSettings rotSet2;

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

    rotCfg.rotRefresh = configFile.value("rotRefresh", 1).toInt();
    rotCfg.udp = configFile.value("udp", false).toBool();
    rotCfg.udpAddress = configFile.value("udpAddress", "127.0.0.1").toString();
    rotCfg.udpPort = configFile.value("udpPort", 12000).toUInt();   //should be toUShort()

    std::copy(defaultPreset, defaultPreset+9, rotCfg.preset);
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

    delete ui;
}

//* Get values
void MainWindow::rotUpdate()
{
    rotDaemon->rotUpdate(my_rot, &rotGet);
    if (rotSet2.enable) rotDaemon->rotUpdate(my_rot2, &rotGet2);
}

//* RotDaemon handle results
void MainWindow::on_rotDaemonResultReady()
{
    guiUpdate();
}

void MainWindow::guiInit()
{
    ui->tabWidget_rotator->setTabText(0, rotSet.nameLabel);
    ui->spinBox_posAz->setMaximum(my_rot->caps->max_az);
    ui->spinBox_posAz->setMinimum(my_rot->caps->min_az);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ui->tabWidget_rotator->removeTab(3);
    ui->tabWidget_rotator->removeTab(2);
#endif

    if (rotSet2.enable)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ui->tabWidget_rotator->setTabVisible(1, true);
#endif
        ui->tabWidget_rotator->setTabText(1, rotSet2.nameLabel);
        ui->spinBox_posAz_2->setMaximum(my_rot2->caps->max_az);
        ui->spinBox_posAz_2->setMinimum(my_rot2->caps->min_az);
    }
    else
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ui->tabWidget_rotator->removeTab(1);
#endif
    }
}

void MainWindow::guiUpdate()
{
    ui->lcdNumber_posAz->display(rotGet.az);

    if (rotSet2.enable)
    {
        ui->lcdNumber_posAz_2->display(rotGet2.az);
    }

    if (rotUdpEx.azUdpFlag || rotUdpEx.elUdpFlag)
    {
        rotUdpEx.azUdpFlag = false;
        rotUdpEx.elUdpFlag = false;

        switch (ui->tabWidget_rotator->currentIndex())
        {
        case 0:
            rotSet.az = rotUdpEx.azUdp;
            rotSet.el = rotUdpEx.elUdp;
            ui->spinBox_posAz->setValue(rotSet.az);
            rot_set_position(my_rot, rotSet.az, rotSet.el);
            break;
        case 1:
            rotSet2.az = rotUdpEx.azUdp;
            rotSet2.el = rotUdpEx.elUdp;
            ui->spinBox_posAz_2->setValue(rotSet2.az);
            rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
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
        rotSet.az = rotCfg.preset[presetNumber];
        ui->spinBox_posAz->setValue(rotSet.az);
        rot_set_position(my_rot, rotSet.az, rotSet.el);
        break;
    case 1:
        rotSet2.az = rotCfg.preset[presetNumber];
        ui->spinBox_posAz_2->setValue(rotSet2.az);
        rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
        break;
    }
}

//* Buttons
void MainWindow::on_pushButton_connect_toggled(bool checked)
{
    if (checked && rotCom.connected == 0)
    {
       my_rot = rotDaemon->rotConnect(&rotCom);   //Open Rotator connection
       if (rotSet2.enable) my_rot2 = rotDaemon->rotConnect(&rotCom2);

       if (!my_rot)   //Connection error
       {
           rotCom.connected = 0;
           //ui->statusbar->showMessage(rigerror(retcode));
           ui->pushButton_connect->setChecked(false);  //Uncheck the button
       }
       else    //Rotator connected
       {
           rotCom.connected = 1;
           //ui->statusbar->showMessage(my_rot->caps->model_name);
           timer->start(rotCfg.rotRefresh*1000);
           guiInit();
       }

       if (!my_rot2) rotCom2.connected = 0;
       else rotCom2.connected = 1;

    }
    else if (rotCom.connected)   //Button unchecked
    {
        rotCom.connected = 0;
        timer->stop();
        rot_close(my_rot);  //Close the communication to the rotator

        if (rotSet2.enable)
        {
            rotCom2.connected = 1;
            rot_close(my_rot2);
        }
    }
}

void MainWindow::on_pushButton_stop_clicked()
{
    rot_stop(my_rot);
    if (rotSet2.enable) rot_stop(my_rot2);
}

void MainWindow::on_pushButton_go_clicked()
{
    rotSet.az = ui->spinBox_posAz->value();
    rot_set_position(my_rot, rotSet.az, rotSet.el);
}

void MainWindow::on_pushButton_go_2_clicked()
{
    rotSet2.az = ui->spinBox_posAz_2->value();
    rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
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
            ui->spinBox_posAz->setValue(rotSet.az);
            rot_set_position(my_rot, rotSet.az, rotSet.el);
        }
        break;

    case 1:
        if (my_rot2->caps->park) rot_park(my_rot2);
        else
        {
            rotSet2.az = rotSet2.azPark;
            rotSet2.el = rotSet2.elPark;
            ui->spinBox_posAz_2->setValue(rotSet2.az);
            rot_set_position(my_rot2, rotSet2.az, rotSet2.el);
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogrotator.h"
#include "dialogsetup.h"
#include "rotatordata.h"
#include "rotdaemon.h"

#include <QMessageBox>
#include <QThread>
#include <QSettings>

#include <rotator.h>    //Hamlib

extern ROT *my_rot; //Defined in rotdaemon.cpp

extern rotatorConnect rotCom;
extern rotatorSettings rotGet;
extern rotatorSettings rotSet;
extern rotatorConfig rotCfg;
extern rotatorUdpEx rotUdpEx;

int retcode;    //Return code from function

FILE* debugFile;

QThread workerThread;
RotDaemon *rotDaemon = new RotDaemon;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget_rotator->setTabEnabled(1, false);

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
    connect(timer, &QTimer::timeout, rotDaemon, &RotDaemon::rotUpdate);
    connect(rotDaemon, &RotDaemon::resultReady, this, &MainWindow::on_rotDaemonResultReady);
    workerThread.start();

    //* Load settings from catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    rotCom.rotModel = configFile.value("Rotator1/rotModel", 0).toInt();
    rotCom.rotPort = configFile.value("Rotator1/rotPort").toString();
    rotCom.serialSpeed = configFile.value("Rotator1/serialSpeed", 9600).toInt();
    rotCom.netRotctl = configFile.value("Rotator1/netRotctl", false).toBool();
    rotSet.nameLabel = configFile.value("Rotator1/nameLabel", "Rotator 1").toString();
    rotSet.azPark = configFile.value("Rotator1/azPark", 0).toInt();
    rotSet.elPark = configFile.value("Rotator1/elPark", 0).toInt();
    rotCom.rotRefresh = configFile.value("rotRefresh", 1).toInt();
    rotCfg.udp = configFile.value("udp", false).toBool();
    rotCfg.udpAddress = configFile.value("udpAddress", "127.0.0.1").toString();
    rotCfg.udpPort = configFile.value("udpPort", 12000).toUInt();   //should be toUShort()
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


//* RotDaemon handle results
void MainWindow::on_rotDaemonResultReady()
{
    guiUpdate();
}

void MainWindow::guiInit()
{
    ui->tabWidget_rotator->setTabText(0, rotSet.nameLabel);

}

void MainWindow::guiUpdate()
{
    ui->lcdNumber_posAz->display(rotGet.az);

    if (rotUdpEx.azUdpFlag)
    {
        rotUdpEx.azUdpFlag = false;
        rotSet.az = rotUdpEx.azUdp;
        ui->spinBox_posAz->setValue(rotSet.az);
        rot_set_position(my_rot, rotSet.az, rotSet.el);
    }

}


//* Buttons
void MainWindow::on_pushButton_connect_toggled(bool checked)
{
    if (checked && rotCom.connected == 0)
    {
       retcode = rotDaemon->rotConnect();   //Open Rig connection

       if (retcode != RIG_OK)   //Connection error
       {
           rotCom.connected = 0;
           ui->statusbar->showMessage(rigerror(retcode));
           ui->pushButton_connect->setChecked(false);  //Uncheck the button
       }
       else    //Rotator connected
       {
           rotCom.connected = 1;
           ui->statusbar->showMessage(my_rot->caps->model_name);
           timer->start(rotCom.rotRefresh*1000);
           guiInit();
       }
    }
    else if (rotCom.connected)   //Button unchecked
    {
        rotCom.connected = 0;
        timer->stop();
        rot_close(my_rot);  //Close the communication to the rig
    }

}

void MainWindow::on_pushButton_park_clicked()
{
    if (my_rot->caps->park) rot_park(my_rot);
    else
    {
        rotSet.az = rotSet.azPark;
        rotSet.el = rotSet.elPark;
        ui->spinBox_posAz->setValue(rotSet.az);
        rot_set_position(my_rot, rotSet.az, rotSet.el);
    }
}

void MainWindow::on_pushButton_stop_clicked()
{
    rot_stop(my_rot);
}

void MainWindow::on_pushButton_go_clicked()
{
    rotSet.az = ui->spinBox_posAz->value();
    rot_set_position(my_rot, rotSet.az, rotSet.el);
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

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


#include "dialogsetup.h"
#include "ui_dialogsetup.h"
#include "rotatordata.h"

#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QDir>

extern catRotatorConfig rotCfg;


DialogSetup::DialogSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetup)
{
    ui->setupUi(this);

    ui->lineEdit_qthLocator->setText(rotCfg.qthLocator);
    ui->checkBox_udp->setChecked(rotCfg.udp);
    ui->lineEdit_udpAddress->setText(rotCfg.udpAddress);
    ui->lineEdit_udpPort->setText(QString::number(rotCfg.udpPort));
    ui->lineEdit_AirScout->setText(rotCfg.pathTrackAirScout);
}

DialogSetup::~DialogSetup()
{
    delete ui;
}

void DialogSetup::on_pushButton_AirScout_clicked()
{
    //QString fileTrackAirScout = QFileDialog::getOpenFileName(this, "Set file", rotCfg.pathTrackAirScout);
    rotCfg.pathTrackAirScout = QFileDialog::getExistingDirectory(this, "Set path", rotCfg.pathTrackAirScout);
    //fileTrackAirScout = fileTrackAirScout + "/azel.dat";
    ui->lineEdit_AirScout->setText(rotCfg.pathTrackAirScout);
}

void DialogSetup::on_buttonBox_accepted()
{
    rotCfg.qthLocator = ui->lineEdit_qthLocator->text();
    rotCfg.udp = ui->checkBox_udp->checkState();
    rotCfg.udpAddress = ui->lineEdit_udpAddress->text();
    rotCfg.udpPort = ui->lineEdit_udpPort->text().toUShort();
    rotCfg.pathTrackAirScout = ui->lineEdit_AirScout->text();

    //* Save settings in catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    configFile.setValue("qthLocator", rotCfg.qthLocator);
    configFile.setValue("udp", rotCfg.udp);
    configFile.setValue("udpAddress", rotCfg.udpAddress);
    configFile.setValue("udpPort", rotCfg.udpPort);
    configFile.setValue("pathTrackAirScout", rotCfg.pathTrackAirScout);
}

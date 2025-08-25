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


#include "dialogsetup.h"
#include "ui_dialogsetup.h"
#include "rotatordata.h"

#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

extern catRotatorConfig rotCfg;


DialogSetup::DialogSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetup)
{
    ui->setupUi(this);

    //* Set config
    ui->lineEdit_qthLocator->setText(rotCfg.qthLocator);
    ui->radioButton_mi->setChecked(rotCfg.distance);
    ui->checkBox_udp->setChecked(rotCfg.udp);
    ui->lineEdit_udpAddress->setText(rotCfg.udpAddress);
    ui->lineEdit_udpPort->setText(QString::number(rotCfg.udpPort));
    ui->lineEdit_WSJTXStatus->setText(rotCfg.pathTrackWSJTXStatus);
    ui->lineEdit_WSJTX->setText(rotCfg.pathTrackWSJTX);
    ui->lineEdit_AirScout->setText(rotCfg.pathTrackAirScout);
    ui->radioButton_themeDark->setChecked(rotCfg.darkTheme);
    ui->checkBox_autoConnect->setChecked(rotCfg.autoConnect);
    ui->checkBox_debug->setChecked(rotCfg.debugMode);
}

DialogSetup::~DialogSetup()
{
    delete ui;
}

void DialogSetup::on_pushButton_udpDefault_clicked()    //Set default UDP address and port
{
    ui->lineEdit_udpAddress->setText("127.0.0.1");
    ui->lineEdit_udpPort->setText("12000");
}

void DialogSetup::on_pushButton_AirScout_clicked()
{
    //QString fileTrackAirScout = QFileDialog::getOpenFileName(this, "Set file", rotCfg.pathTrackAirScout);
    rotCfg.pathTrackAirScout = QFileDialog::getExistingDirectory(this, "Set path", rotCfg.pathTrackAirScout);
    //fileTrackAirScout = fileTrackAirScout + "/azel.dat";
    ui->lineEdit_AirScout->setText(rotCfg.pathTrackAirScout);
}

void DialogSetup::on_pushButton_WSJTXStatus_clicked()
{
    rotCfg.pathTrackWSJTXStatus = QFileDialog::getExistingDirectory(this, "Set path", rotCfg.pathTrackWSJTXStatus);
    ui->lineEdit_WSJTXStatus->setText(rotCfg.pathTrackWSJTXStatus);
}

void DialogSetup::on_pushButton_WSJTX_clicked()
{
    rotCfg.pathTrackWSJTX = QFileDialog::getExistingDirectory(this, "Set path", rotCfg.pathTrackWSJTX);
    ui->lineEdit_WSJTX->setText(rotCfg.pathTrackWSJTX);
}

void DialogSetup::on_buttonBox_accepted()
{
    //MessageBox to restart if UDP config is changed
    if ((rotCfg.udp != ui->checkBox_udp->isChecked()) || (rotCfg.udpAddress != ui->lineEdit_udpAddress->text()) || (rotCfg.udpPort != ui->lineEdit_udpPort->text().toUShort()))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("UDP");
        msgBox.setText("Please, restart CatRotator to make effective the UDP setup changes.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    if ((rotCfg.darkTheme != ui->radioButton_themeDark->isChecked()))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Theme");
        msgBox.setText("Please, restart CatRotator to make effective the theme.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    //* Load config from dialog
    rotCfg.qthLocator = ui->lineEdit_qthLocator->text();
    rotCfg.distance = ui->radioButton_mi->isChecked();
    rotCfg.udp = ui->checkBox_udp->isChecked();
    rotCfg.udpAddress = ui->lineEdit_udpAddress->text();
    rotCfg.udpPort = ui->lineEdit_udpPort->text().toUShort();
    rotCfg.pathTrackWSJTXStatus = ui->lineEdit_WSJTXStatus->text();
    rotCfg.pathTrackWSJTX = ui->lineEdit_WSJTX->text();
    rotCfg.pathTrackAirScout = ui->lineEdit_AirScout->text();
    rotCfg.darkTheme = ui->radioButton_themeDark->isChecked();
    rotCfg.autoConnect = ui->checkBox_autoConnect->isChecked();
    rotCfg.debugMode = ui->checkBox_debug->isChecked();

    //* Save settings in catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    configFile.setValue("qthLocator", rotCfg.qthLocator);
    configFile.setValue("distance", rotCfg.distance);
    configFile.setValue("udp", rotCfg.udp);
    configFile.setValue("udpAddress", rotCfg.udpAddress);
    configFile.setValue("udpPort", rotCfg.udpPort);
    configFile.setValue("pathTrackWSJTXStatus", rotCfg.pathTrackWSJTXStatus);
    configFile.setValue("pathTrackWSJTX", rotCfg.pathTrackWSJTX);
    configFile.setValue("pathTrackAirScout", rotCfg.pathTrackAirScout);
    configFile.setValue("darkTheme", rotCfg.darkTheme);
    configFile.setValue("autoConnect", rotCfg.autoConnect);
    configFile.setValue("debugMode", rotCfg.debugMode);
}

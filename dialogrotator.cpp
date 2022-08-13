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


#include "dialogrotator.h"
#include "ui_dialogrotator.h"

#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QSerialPortInfo>
#include <QMessageBox>

#include "rotatordata.h"

#include <rotator.h>


extern rotatorConnect rotCom;
extern rotatorSettings rotSet;

extern rotatorConnect rotCom2;
extern rotatorSettings rotSet2;

extern rotatorConnect rotCom3;
extern rotatorSettings rotSet3;

extern catRotatorConfig rotCfg;


QString rotListFile = "rotator.lst";    //Text file containing the list of rotators supported by hamlib
QFile file(rotListFile);


DialogRotator::DialogRotator(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRotator)
{
    ui->setupUi(this);

    //* rigModel comboBox
    if (!file.exists()) //Create file rotator.lst if not exists
    {
        file.open(QIODevice::ReadWrite);
        rot_load_all_backends();    //Load all backends information
        rot_list_foreach(printRotatorList, NULL);   //Create the rotators list
    }
    else file.open(QIODevice::ReadOnly);    //Open file rotators.lst and populate the combobox
    file.seek(0);
    ui->comboBox_rotModel->clear();
    ui->comboBox_rotModel->addItem("");
    QRegularExpression regexp("[0-9]+");
    while(!file.atEnd())
    {
        QString line = file.readLine();
        int i = 1;
        if (ui->comboBox_rotModel->count() == 1) ui->comboBox_rotModel->addItem(line.trimmed());    //first line
        else while (i < ui->comboBox_rotModel->count()) //sort ascending by model number
        {
            QRegularExpressionMatch rotNumberNew = regexp.match(line);
            ui->comboBox_rotModel->setCurrentIndex(i);
            QString rotModel = ui->comboBox_rotModel->currentText();
            QRegularExpressionMatch rotNumber = regexp.match(rotModel);
            if (rotNumberNew.captured(0).toInt() < rotNumber.captured(0).toInt())
            {
                ui->comboBox_rotModel->insertItem(i, line.trimmed());
                break;
            }
            else i++;
        }
        if (i == ui->comboBox_rotModel->count()) ui->comboBox_rotModel->addItem(line.trimmed());
    }

    file.seek(0);
    ui->comboBox_rotModel_2->clear();
    ui->comboBox_rotModel_2->addItem("");
    while(!file.atEnd())
    {
        QString line = file.readLine();
        int i = 1;
        if (ui->comboBox_rotModel_2->count() == 1) ui->comboBox_rotModel_2->addItem(line.trimmed());    //first line
        else while (i < ui->comboBox_rotModel_2->count()) //sort ascending by model number
        {
            QRegularExpressionMatch rotNumberNew = regexp.match(line);
            ui->comboBox_rotModel_2->setCurrentIndex(i);
            QString rotModel = ui->comboBox_rotModel_2->currentText();
            QRegularExpressionMatch rotNumber = regexp.match(rotModel);
            if (rotNumberNew.captured(0).toInt() < rotNumber.captured(0).toInt())
            {
                ui->comboBox_rotModel_2->insertItem(i, line.trimmed());
                break;
            }
            else i++;
        }
        if (i == ui->comboBox_rotModel_2->count()) ui->comboBox_rotModel_2->addItem(line.trimmed());
    }

    file.seek(0);
    ui->comboBox_rotModel_3->clear();
    ui->comboBox_rotModel_3->addItem("");
    while(!file.atEnd())
    {
        QString line = file.readLine();
        int i = 1;
        if (ui->comboBox_rotModel_3->count() == 1) ui->comboBox_rotModel_3->addItem(line.trimmed());    //first line
        else while (i < ui->comboBox_rotModel_3->count()) //sort ascending by model number
        {
            QRegularExpressionMatch rotNumberNew = regexp.match(line);
            ui->comboBox_rotModel_3->setCurrentIndex(i);
            QString rotModel = ui->comboBox_rotModel_3->currentText();
            QRegularExpressionMatch rotNumber = regexp.match(rotModel);
            if (rotNumberNew.captured(0).toInt() < rotNumber.captured(0).toInt())
            {
                ui->comboBox_rotModel_3->insertItem(i, line.trimmed());
                break;
            }
            else i++;
        }
        if (i == ui->comboBox_rotModel_3->count()) ui->comboBox_rotModel_3->addItem(line.trimmed());
    }
    file.close();

    //* COM port
    ui->comboBox_comPort->clear();
    ui->comboBox_comPort_2->clear();
    ui->comboBox_comPort_3->clear();
    ui->comboBox_comPort->addItem("");
    ui->comboBox_comPort_2->addItem("");
    ui->comboBox_comPort_3->addItem("");
    foreach (const QSerialPortInfo &comPort, QSerialPortInfo::availablePorts())  //search available COM port
    {
#ifdef Q_OS_WIN
        ui->comboBox_comPort->addItem(comPort.portName());
        ui->comboBox_comPort_2->addItem(comPort.portName());
        ui->comboBox_comPort_3->addItem(comPort.portName());
#endif
#ifdef Q_OS_LINUX
        ui->comboBox_comPort->addItem("/dev/"+comPort.portName());
        ui->comboBox_comPort_2->addItem("/dev/"+comPort.portName());
        ui->comboBox_comPort_3->addItem("/dev/"+comPort.portName());
#endif
    }

    //* serialSpeed
    ui->comboBox_serialSpeed->clear();
    ui->comboBox_serialSpeed->addItem("");
    ui->comboBox_serialSpeed->addItem("4800");
    ui->comboBox_serialSpeed->addItem("9600");
    ui->comboBox_serialSpeed->addItem("19200");
    ui->comboBox_serialSpeed->addItem("38400");

    ui->comboBox_serialSpeed_2->clear();
    ui->comboBox_serialSpeed_2->addItem("");
    ui->comboBox_serialSpeed_2->addItem("4800");
    ui->comboBox_serialSpeed_2->addItem("9600");
    ui->comboBox_serialSpeed_2->addItem("19200");
    ui->comboBox_serialSpeed_2->addItem("38400");

    ui->comboBox_serialSpeed_3->clear();
    ui->comboBox_serialSpeed_3->addItem("");
    ui->comboBox_serialSpeed_3->addItem("4800");
    ui->comboBox_serialSpeed_3->addItem("9600");
    ui->comboBox_serialSpeed_3->addItem("19200");
    ui->comboBox_serialSpeed_3->addItem("38400");

    //* Update values in the GUI
    ui->comboBox_rotModel->setCurrentIndex(ui->comboBox_rotModel->findText(QString::number(rotCom.rotModel),Qt::MatchStartsWith));
    if (rotCom.netRotctl)
    {
        ui->checkBox_netRotctl->setChecked(rotCom.netRotctl);
        ui->lineEdit_ip->setText(rotCom.rotPort);
    }
    else
    {
        ui->comboBox_comPort->setCurrentText(rotCom.rotPort);
        ui->comboBox_serialSpeed->setCurrentText(QString::number(rotCom.serialSpeed));
    }

    ui->lineEdit_name->setText(rotSet.nameLabel);
    ui->spinBox_azPark->setValue(rotSet.azPark);
    ui->spinBox_elPark->setValue(rotSet.elPark);
    ui->doubleSpinBox_tolerance->setValue(rotSet.trackTolerance);
    ui->checkBox_WSJTX->setChecked(rotSet.trackWSJTX);
    ui->checkBox_AirScout->setChecked(rotSet.trackAirScout);

    ui->comboBox_rotModel_2->setCurrentIndex(ui->comboBox_rotModel_2->findText(QString::number(rotCom2.rotModel),Qt::MatchStartsWith));
    if (rotCom2.netRotctl)
    {
        ui->checkBox_netRotctl_2->setChecked(rotCom2.netRotctl);
        ui->lineEdit_ip_2->setText(rotCom2.rotPort);
    }
    else
    {
        ui->comboBox_comPort_2->setCurrentText(rotCom2.rotPort);
        ui->comboBox_serialSpeed_2->setCurrentText(QString::number(rotCom2.serialSpeed));
    }

    ui->lineEdit_name_2->setText(rotSet2.nameLabel);
    ui->spinBox_azPark_2->setValue(rotSet2.azPark);
    ui->spinBox_elPark_2->setValue(rotSet2.elPark);
    ui->doubleSpinBox_tolerance_2->setValue(rotSet2.trackTolerance);
    ui->checkBox_WSJTX_2->setChecked(rotSet2.trackWSJTX);
    ui->checkBox_AirScout_2->setChecked(rotSet2.trackAirScout);

    ui->comboBox_rotModel_3->setCurrentIndex(ui->comboBox_rotModel_3->findText(QString::number(rotCom3.rotModel),Qt::MatchStartsWith));
    if (rotCom3.netRotctl)
    {
        ui->checkBox_netRotctl_3->setChecked(rotCom3.netRotctl);
        ui->lineEdit_ip_3->setText(rotCom3.rotPort);
    }
    else
    {
        ui->comboBox_comPort_3->setCurrentText(rotCom3.rotPort);
        ui->comboBox_serialSpeed_3->setCurrentText(QString::number(rotCom3.serialSpeed));
    }

    ui->lineEdit_name_3->setText(rotSet3.nameLabel);
    ui->spinBox_azPark_3->setValue(rotSet3.azPark);
    ui->spinBox_elPark_3->setValue(rotSet3.elPark);
    ui->doubleSpinBox_tolerance_3->setValue(rotSet3.trackTolerance);
    ui->checkBox_WSJTX_3->setChecked(rotSet3.trackWSJTX);
    ui->checkBox_AirScout_3->setChecked(rotSet3.trackAirScout);

    ui->spinBox_refreshRate->setValue(rotCfg.rotRefresh);
    ui->spinBox_incrementAz->setValue(rotCfg.incrementAz);
}

DialogRotator::~DialogRotator()
{
    delete ui;
}

void DialogRotator::on_buttonBox_accepted()
{
    //* Read settings from GUI
    if (ui->comboBox_rotModel->currentText() == "") //No backend selected
    {
        QMessageBox msgBox; //Show error MessageBox
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Rotator model not selected");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
    else
    {
        QString rotModel = ui->comboBox_rotModel->currentText();
        QRegularExpression regexp("[0-9]+");
        QRegularExpressionMatch rotNumber = regexp.match(rotModel);
        rotCom.rotModel = rotNumber.captured(0).toInt();

        if (ui->checkBox_netRotctl->isChecked())   //TCP port
        {
            rotCom.netRotctl = true;
            rotCom.rotPort = ui->lineEdit_ip->text();

            if (rotCom.rotPort == "")
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rotModel + "\nIP address not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
        else    //COM port
        {
            rotCom.netRotctl = false;
            rotCom.rotPort = ui->comboBox_comPort->currentText();
            rotCom.serialSpeed = ui->comboBox_serialSpeed->currentText().toInt();

            if (rotCom.rotPort == "" && rotCom.rotModel != 1)
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rotModel + "\nCOM port not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
    }

    rotSet.nameLabel = ui->lineEdit_name->text();
    rotSet.azPark = ui->spinBox_azPark->value();
    rotSet.elPark = ui->spinBox_elPark->value();
    rotSet.trackTolerance = ui->doubleSpinBox_tolerance->value();
    rotSet.trackWSJTX = ui->checkBox_WSJTX->isChecked();
    rotSet.trackAirScout = ui->checkBox_AirScout->isChecked();

    //Rotator 2
    if (ui->comboBox_rotModel_2->currentText() == "") //No backend selected
    {
        rotSet2.enable = false;
        rotCom2.rotModel = 0;
    }
    else
    {
        rotSet2.enable = true;
        QString rotModel = ui->comboBox_rotModel_2->currentText();
        QRegularExpression regexp("[0-9]+");
        QRegularExpressionMatch rotNumber = regexp.match(rotModel);
        rotCom2.rotModel = rotNumber.captured(0).toInt();

        if (ui->checkBox_netRotctl_2->isChecked())   //TCP port
        {
            rotCom2.netRotctl = true;
            rotCom2.rotPort = ui->lineEdit_ip_2->text();

            if (rotCom2.rotPort == "")
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rotModel + "\nIP address not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
        else    //COM port
        {
            rotCom2.netRotctl = false;
            rotCom2.rotPort = ui->comboBox_comPort_2->currentText();
            rotCom2.serialSpeed = ui->comboBox_serialSpeed_2->currentText().toInt();

            if (rotCom2.rotPort == "" && rotCom2.rotModel != 1)
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rotModel + "\nCOM port not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
    }

    rotSet2.nameLabel = ui->lineEdit_name_2->text();
    rotSet2.azPark = ui->spinBox_azPark_2->value();
    rotSet2.elPark = ui->spinBox_elPark_2->value();
    rotSet2.trackTolerance = ui->doubleSpinBox_tolerance_2->value();
    rotSet2.trackWSJTX = ui->checkBox_WSJTX_2->isChecked();
    rotSet2.trackAirScout = ui->checkBox_AirScout_2->isChecked();

    //Rotator 3
    if (ui->comboBox_rotModel_3->currentText() == "") //No backend selected
    {
        rotSet3.enable = false;
        rotCom3.rotModel = 0;
    }
    else
    {
        rotSet3.enable = true;
        QString rotModel = ui->comboBox_rotModel_3->currentText();
        QRegularExpression regexp("[0-9]+");
        QRegularExpressionMatch rotNumber = regexp.match(rotModel);
        rotCom3.rotModel = rotNumber.captured(0).toInt();

        if (ui->checkBox_netRotctl_3->isChecked())   //TCP port
        {
            rotCom3.netRotctl = true;
            rotCom3.rotPort = ui->lineEdit_ip_3->text();

            if (rotCom3.rotPort == "")
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rotModel + "\nIP address not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
        else    //COM port
        {
            rotCom3.netRotctl = false;
            rotCom3.rotPort = ui->comboBox_comPort_3->currentText();
            rotCom3.serialSpeed = ui->comboBox_serialSpeed_3->currentText().toInt();

            if (rotCom3.rotPort == "" && rotCom3.rotModel != 1)
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rotModel + "\nCOM port not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
    }

    rotSet3.nameLabel = ui->lineEdit_name_3->text();
    rotSet3.azPark = ui->spinBox_azPark_3->value();
    rotSet3.elPark = ui->spinBox_elPark_3->value();
    rotSet3.trackTolerance = ui->doubleSpinBox_tolerance_3->value();
    rotSet3.trackWSJTX = ui->checkBox_WSJTX_3->isChecked();
    rotSet3.trackAirScout = ui->checkBox_AirScout_3->isChecked();

    rotCfg.rotRefresh = ui->spinBox_refreshRate->value();   //Refresh
    rotCfg.incrementAz = ui->spinBox_incrementAz->value();  //Increment

    //* Save settings in catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    configFile.setValue("Rotator1/rotModel", rotCom.rotModel);
    configFile.setValue("Rotator1/rotPort", rotCom.rotPort);
    configFile.setValue("Rotator1/serialSpeed", ui->comboBox_serialSpeed->currentText());
    configFile.setValue("Rotator1/netRotctl", ui->checkBox_netRotctl->isChecked());
    configFile.setValue("Rotator1/nameLabel", rotSet.nameLabel);
    configFile.setValue("Rotator1/azPark", rotSet.azPark);
    configFile.setValue("Rotator1/elPark", rotSet.elPark);
    configFile.setValue("Rotator1/trackTolerance", rotSet.trackTolerance);
    configFile.setValue("Rotator1/trackWSJTX", rotSet.trackWSJTX);
    configFile.setValue("Rotator1/trackAirScout", rotSet.trackAirScout);

    configFile.setValue("Rotator2/rotModel", rotCom2.rotModel);
    configFile.setValue("Rotator2/rotPort", rotCom2.rotPort);
    configFile.setValue("Rotator2/serialSpeed", ui->comboBox_serialSpeed_2->currentText());
    configFile.setValue("Rotator2/netRotctl", ui->checkBox_netRotctl_2->isChecked());
    configFile.setValue("Rotator2/nameLabel", rotSet2.nameLabel);
    configFile.setValue("Rotator2/azPark", rotSet2.azPark);
    configFile.setValue("Rotator2/elPark", rotSet2.elPark);
    configFile.setValue("Rotator2/trackTolerance", rotSet2.trackTolerance);
    configFile.setValue("Rotator2/trackWSJTX", rotSet2.trackWSJTX);
    configFile.setValue("Rotator2/trackAirScout", rotSet2.trackAirScout);

    configFile.setValue("Rotator3/rotModel", rotCom3.rotModel);
    configFile.setValue("Rotator3/rotPort", rotCom3.rotPort);
    configFile.setValue("Rotator3/serialSpeed", ui->comboBox_serialSpeed_3->currentText());
    configFile.setValue("Rotator3/netRotctl", ui->checkBox_netRotctl_3->isChecked());
    configFile.setValue("Rotator3/nameLabel", rotSet3.nameLabel);
    configFile.setValue("Rotator3/azPark", rotSet3.azPark);
    configFile.setValue("Rotator3/elPark", rotSet3.elPark);
    configFile.setValue("Rotator3/trackTolerance", rotSet3.trackTolerance);
    configFile.setValue("Rotator3/trackWSJTX", rotSet3.trackWSJTX);
    configFile.setValue("Rotator3/trackAirScout", rotSet3.trackAirScout);

    configFile.setValue("rotRefresh", ui->spinBox_refreshRate->value());
    configFile.setValue("rotIncrementAz", ui->spinBox_incrementAz->value());
}

int printRotatorList(const struct rot_caps *rotCaps, void *data)    //Load rotators list from hamlib and save into file rotator.lst
{
    if (data) return 0;
    QTextStream stream(&file);
    stream << rotCaps->rot_model << " " << rotCaps->mfg_name << " " << rotCaps->model_name << "\n";
    return 1;
}

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


extern rotatorConnect rotCom[3];
extern rotatorSettings rotSet[3];
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
    ui->comboBox_rotModel->setCurrentIndex(ui->comboBox_rotModel->findText(QString::number(rotCom[0].rotModel),Qt::MatchStartsWith));
    if (rotCom[0].netRotctl)
    {
        ui->checkBox_netRotctl->setChecked(rotCom[0].netRotctl);
        ui->lineEdit_ip->setText(rotCom[0].rotPort);
    }
    else
    {
        ui->comboBox_comPort->setCurrentText(rotCom[0].rotPort);
        ui->comboBox_serialSpeed->setCurrentText(QString::number(rotCom[0].serialSpeed));
    }

    ui->lineEdit_name->setText(rotSet[0].nameLabel);
    ui->spinBox_azPark->setValue(rotSet[0].azPark);
    ui->spinBox_elPark->setValue(rotSet[0].elPark);
    ui->checkBox_overlap->setChecked(rotSet[0].overlap);
    ui->doubleSpinBox_tolerance->setValue(rotSet[0].trackTolerance);
    ui->doubleSpinBox_threshold->setValue(rotSet[0].trackThreshold);
    ui->checkBox_PreviSat->setChecked(rotSet[0].trackPreviSat);
    ui->checkBox_WSJTX->setChecked(rotSet[0].trackWSJTX);
    ui->checkBox_AirScout->setChecked(rotSet[0].trackAirScout);

    ui->comboBox_rotModel_2->setCurrentIndex(ui->comboBox_rotModel_2->findText(QString::number(rotCom[1].rotModel),Qt::MatchStartsWith));
    if (rotCom[1].netRotctl)
    {
        ui->checkBox_netRotctl_2->setChecked(rotCom[1].netRotctl);
        ui->lineEdit_ip_2->setText(rotCom[1].rotPort);
    }
    else
    {
        ui->comboBox_comPort_2->setCurrentText(rotCom[1].rotPort);
        ui->comboBox_serialSpeed_2->setCurrentText(QString::number(rotCom[1].serialSpeed));
    }

    ui->lineEdit_name_2->setText(rotSet[1].nameLabel);
    ui->spinBox_azPark_2->setValue(rotSet[1].azPark);
    ui->spinBox_elPark_2->setValue(rotSet[1].elPark);
    ui->checkBox_overlap_2->setChecked(rotSet[1].overlap);
    ui->doubleSpinBox_tolerance_2->setValue(rotSet[1].trackTolerance);
    ui->doubleSpinBox_threshold_2->setValue(rotSet[1].trackThreshold);
    ui->checkBox_PreviSat_2->setChecked(rotSet[1].trackPreviSat);
    ui->checkBox_WSJTX_2->setChecked(rotSet[1].trackWSJTX);
    ui->checkBox_AirScout_2->setChecked(rotSet[1].trackAirScout);

    ui->comboBox_rotModel_3->setCurrentIndex(ui->comboBox_rotModel_3->findText(QString::number(rotCom[2].rotModel),Qt::MatchStartsWith));
    if (rotCom[2].netRotctl)
    {
        ui->checkBox_netRotctl_3->setChecked(rotCom[2].netRotctl);
        ui->lineEdit_ip_3->setText(rotCom[2].rotPort);
    }
    else
    {
        ui->comboBox_comPort_3->setCurrentText(rotCom[2].rotPort);
        ui->comboBox_serialSpeed_3->setCurrentText(QString::number(rotCom[2].serialSpeed));
    }

    ui->lineEdit_name_3->setText(rotSet[2].nameLabel);
    ui->spinBox_azPark_3->setValue(rotSet[2].azPark);
    ui->spinBox_elPark_3->setValue(rotSet[2].elPark);
    ui->checkBox_overlap_3->setChecked(rotSet[2].overlap);
    ui->doubleSpinBox_tolerance_3->setValue(rotSet[2].trackTolerance);
    ui->doubleSpinBox_threshold_3->setValue(rotSet[2].trackThreshold);
    ui->checkBox_PreviSat_3->setChecked(rotSet[2].trackPreviSat);
    ui->checkBox_WSJTX_3->setChecked(rotSet[2].trackWSJTX);
    ui->checkBox_AirScout_3->setChecked(rotSet[2].trackAirScout);

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
        rotCom[0].rotModel = rotNumber.captured(0).toInt();

        if (ui->checkBox_netRotctl->isChecked())   //TCP port
        {
            rotCom[0].netRotctl = true;
            rotCom[0].rotPort = ui->lineEdit_ip->text();

            if (rotCom[0].rotPort == "")
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
            rotCom[0].netRotctl = false;
            rotCom[0].rotPort = ui->comboBox_comPort->currentText();
            rotCom[0].serialSpeed = ui->comboBox_serialSpeed->currentText().toInt();

            if (rotCom[0].rotPort == "" && rotCom[0].rotModel != 1)
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

    rotSet[0].nameLabel = ui->lineEdit_name->text();
    rotSet[0].azPark = ui->spinBox_azPark->value();
    rotSet[0].elPark = ui->spinBox_elPark->value();
    rotSet[0].overlap = ui->checkBox_overlap->isChecked();
    rotSet[0].trackTolerance = ui->doubleSpinBox_tolerance->value();
    rotSet[0].trackThreshold = ui->doubleSpinBox_threshold->value();
    rotSet[0].trackPreviSat = ui->checkBox_PreviSat->isChecked();
    rotSet[0].trackWSJTX = ui->checkBox_WSJTX->isChecked();
    rotSet[0].trackAirScout = ui->checkBox_AirScout->isChecked();

    //Rotator 2
    if (ui->comboBox_rotModel_2->currentText() == "") //No backend selected
    {
        rotSet[1].enable = false;
        rotCom[1].rotModel = 0;
    }
    else
    {
        rotSet[1].enable = true;
        QString rotModel = ui->comboBox_rotModel_2->currentText();
        QRegularExpression regexp("[0-9]+");
        QRegularExpressionMatch rotNumber = regexp.match(rotModel);
        rotCom[1].rotModel = rotNumber.captured(0).toInt();

        if (ui->checkBox_netRotctl_2->isChecked())   //TCP port
        {
            rotCom[1].netRotctl = true;
            rotCom[1].rotPort = ui->lineEdit_ip_2->text();

            if (rotCom[1].rotPort == "")
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
            rotCom[1].netRotctl = false;
            rotCom[1].rotPort = ui->comboBox_comPort_2->currentText();
            rotCom[1].serialSpeed = ui->comboBox_serialSpeed_2->currentText().toInt();

            if (rotCom[1].rotPort == "" && rotCom[1].rotModel != 1)
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

    rotSet[1].nameLabel = ui->lineEdit_name_2->text();
    rotSet[1].azPark = ui->spinBox_azPark_2->value();
    rotSet[1].elPark = ui->spinBox_elPark_2->value();
    rotSet[1].overlap = ui->checkBox_overlap_2->isChecked();
    rotSet[1].trackTolerance = ui->doubleSpinBox_tolerance_2->value();
    rotSet[1].trackThreshold = ui->doubleSpinBox_threshold_2->value();
    rotSet[1].trackPreviSat = ui->checkBox_PreviSat_2->isChecked();
    rotSet[1].trackWSJTX = ui->checkBox_WSJTX_2->isChecked();
    rotSet[1].trackAirScout = ui->checkBox_AirScout_2->isChecked();

    //Rotator 3
    if (ui->comboBox_rotModel_3->currentText() == "") //No backend selected
    {
        rotSet[2].enable = false;
        rotCom[2].rotModel = 0;
    }
    else
    {
        rotSet[2].enable = true;
        QString rotModel = ui->comboBox_rotModel_3->currentText();
        QRegularExpression regexp("[0-9]+");
        QRegularExpressionMatch rotNumber = regexp.match(rotModel);
        rotCom[2].rotModel = rotNumber.captured(0).toInt();

        if (ui->checkBox_netRotctl_3->isChecked())   //TCP port
        {
            rotCom[2].netRotctl = true;
            rotCom[2].rotPort = ui->lineEdit_ip_3->text();

            if (rotCom[2].rotPort == "")
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
            rotCom[2].netRotctl = false;
            rotCom[2].rotPort = ui->comboBox_comPort_3->currentText();
            rotCom[2].serialSpeed = ui->comboBox_serialSpeed_3->currentText().toInt();

            if (rotCom[2].rotPort == "" && rotCom[2].rotModel != 1)
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

    rotSet[2].nameLabel = ui->lineEdit_name_3->text();
    rotSet[2].azPark = ui->spinBox_azPark_3->value();
    rotSet[2].elPark = ui->spinBox_elPark_3->value();
    rotSet[2].overlap = ui->checkBox_overlap_3->isChecked();
    rotSet[2].trackTolerance = ui->doubleSpinBox_tolerance_3->value();
    rotSet[2].trackThreshold = ui->doubleSpinBox_threshold_3->value();
    rotSet[2].trackPreviSat = ui->checkBox_PreviSat_3->isChecked();
    rotSet[2].trackWSJTX = ui->checkBox_WSJTX_3->isChecked();
    rotSet[2].trackAirScout = ui->checkBox_AirScout_3->isChecked();

    rotCfg.rotRefresh = ui->spinBox_refreshRate->value();   //Refresh
    rotCfg.incrementAz = ui->spinBox_incrementAz->value();  //Increment

    //* Save settings in catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    for (int i = 0; i < 3; i++)
    {
        configFile.setValue("Rotator"+QString::number(i+1)+"/rotModel", rotCom[i].rotModel);
        configFile.setValue("Rotator"+QString::number(i+1)+"/rotPort", rotCom[i].rotPort);
        configFile.setValue("Rotator"+QString::number(i+1)+"/serialSpeed", rotCom[i].serialSpeed);
        configFile.setValue("Rotator"+QString::number(i+1)+"/netRotctl", rotCom[i].netRotctl);
        configFile.setValue("Rotator"+QString::number(i+1)+"/nameLabel", rotSet[i].nameLabel);
        configFile.setValue("Rotator"+QString::number(i+1)+"/azPark", rotSet[i].azPark);
        configFile.setValue("Rotator"+QString::number(i+1)+"/elPark", rotSet[i].elPark);
        configFile.setValue("Rotator"+QString::number(i+1)+"/overlap", rotSet[i].overlap);
        configFile.setValue("Rotator"+QString::number(i+1)+"/trackTolerance", rotSet[i].trackTolerance);
        configFile.setValue("Rotator"+QString::number(i+1)+"/trackThreshold", rotSet[i].trackThreshold);
        configFile.setValue("Rotator"+QString::number(i+1)+"/trackPreviSat", rotSet[i].trackPreviSat);
        configFile.setValue("Rotator"+QString::number(i+1)+"/trackWSJTX", rotSet[i].trackWSJTX);
        configFile.setValue("Rotator"+QString::number(i+1)+"/trackAirScout", rotSet[i].trackAirScout);
    }
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


void DialogRotator::on_checkBox_PreviSat_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_WSJTX->setChecked(false);
        ui->checkBox_AirScout->setChecked(false);
    }
}


void DialogRotator::on_checkBox_WSJTX_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_PreviSat->setChecked(false);
        ui->checkBox_AirScout->setChecked(false);
    }
}


void DialogRotator::on_checkBox_AirScout_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_PreviSat->setChecked(false);
        ui->checkBox_WSJTX->setChecked(false);
    }
}


void DialogRotator::on_checkBox_PreviSat_2_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_WSJTX_2->setChecked(false);
        ui->checkBox_AirScout_2->setChecked(false);
    }
}


void DialogRotator::on_checkBox_WSJTX_2_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_PreviSat_2->setChecked(false);
        ui->checkBox_AirScout_2->setChecked(false);
    }
}


void DialogRotator::on_checkBox_AirScout_2_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_PreviSat_2->setChecked(false);
        ui->checkBox_WSJTX_2->setChecked(false);
    }
}


void DialogRotator::on_checkBox_PreviSat_3_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_WSJTX_3->setChecked(false);
        ui->checkBox_AirScout_3->setChecked(false);
    }
}


void DialogRotator::on_checkBox_WSJTX_3_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_PreviSat_3->setChecked(false);
        ui->checkBox_AirScout_3->setChecked(false);
    }
}


void DialogRotator::on_checkBox_AirScout_3_toggled(bool checked)
{
    if (checked)
    {
        ui->checkBox_PreviSat_3->setChecked(false);
        ui->checkBox_WSJTX_3->setChecked(false);
    }
}


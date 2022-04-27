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
                ui->comboBox_rotModel->insertItem(i,line.trimmed());
                break;
            }
            else i++;
        }
        if (i == ui->comboBox_rotModel->count()) ui->comboBox_rotModel->addItem(line.trimmed());
    }
    file.close();

    //* COM port
    ui->comboBox_comPort->clear();
    ui->comboBox_comPort->addItem("");
    foreach (const QSerialPortInfo &comPort, QSerialPortInfo::availablePorts())  //search available COM port
    {
#ifdef Q_OS_WIN
        ui->comboBox_comPort->addItem(comPort.portName());
#endif
#ifdef Q_OS_LINUX
        ui->comboBox_comPort->addItem("/dev/"+comPort.portName());
#endif
    }

    //* serialSpeed
    ui->comboBox_serialSpeed->clear();
    ui->comboBox_serialSpeed->addItem("");
    ui->comboBox_serialSpeed->addItem("4800");
    ui->comboBox_serialSpeed->addItem("9600");
    ui->comboBox_serialSpeed->addItem("19200");
    ui->comboBox_serialSpeed->addItem("38400");

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
    ui->spinBox_refreshRate->setValue(rotCom.rotRefresh);

    ui->spinBox_azPark->setValue(rotSet.azPark);
    ui->spinBox_elPark->setValue(rotSet.elPark);
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

            if (rotCom.rotPort == "" && rotCom.rotModel != 1 && rotCom.rotModel != 6)
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

    rotCom.rotRefresh = ui->spinBox_refreshRate->value();
    rotSet.azPark = ui->spinBox_azPark->value();
    rotSet.elPark = ui->spinBox_elPark->value();

    //* Save settings in catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    configFile.setValue("Rotator1/rotModel", rotCom.rotModel);
    configFile.setValue("Rotator1/rotPort", rotCom.rotPort);
    configFile.setValue("Rotator1/serialSpeed", ui->comboBox_serialSpeed->currentText());
    configFile.setValue("Rotator1/netRotctl", ui->checkBox_netRotctl->isChecked());
    configFile.setValue("Rotator1/azPark", rotSet.azPark);
    configFile.setValue("Rotator1/elPark", rotSet.elPark);
    configFile.setValue("rotRefresh", ui->spinBox_refreshRate->value());
}

int printRotatorList(const struct rot_caps *rotCaps, void *data)    //Load rotators list from hamlib and save into file rotator.lst
{
    if (data) return 0;
    QTextStream stream(&file);
    stream << rotCaps->rot_model << " " << rotCaps->mfg_name << " " << rotCaps->model_name << "\n";
    return 1;
}

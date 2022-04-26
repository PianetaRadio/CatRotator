#include "dialogsetup.h"
#include "ui_dialogsetup.h"
#include "rotatordata.h"

#include <QSettings>
#include <QFile>

extern rotatorConfig rotCfg;


DialogSetup::DialogSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetup)
{
    ui->setupUi(this);

    ui->checkBox_udp->setChecked(rotCfg.udp);
    ui->lineEdit_udpAddress->setText(rotCfg.udpAddress);
    ui->lineEdit_udpPort->setText(QString::number(rotCfg.udpPort));
}

DialogSetup::~DialogSetup()
{
    delete ui;
}

void DialogSetup::on_buttonBox_accepted()
{
    rotCfg.udp = ui->checkBox_udp->checkState();
    rotCfg.udpAddress = ui->lineEdit_udpAddress->text();
    rotCfg.udpPort = ui->lineEdit_udpPort->text().toUShort();

    //* Save settings in catrotator.ini
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);
    configFile.setValue("udp", rotCfg.udp);
    configFile.setValue("udpAddress", rotCfg.udpAddress);
    configFile.setValue("udpPort", rotCfg.udpPort);
}

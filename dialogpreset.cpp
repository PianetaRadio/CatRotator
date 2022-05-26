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


#include "dialogpreset.h"
#include "ui_dialogpreset.h"

#include <QSettings>
#include <QFile>
#include <QString>

#include "rotatordata.h"

extern catRotatorConfig rotCfg;


DialogPreset::DialogPreset(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPreset)
{
    ui->setupUi(this);

    DialogPreset::presetInit();
}

DialogPreset::~DialogPreset()
{
    delete ui;
}

void DialogPreset::presetInit()
{
    ui->lineEdit_Label_0->setText(rotCfg.presetLabel[0]);
    ui->lineEdit_Label_1->setText(rotCfg.presetLabel[1]);
    ui->lineEdit_Label_2->setText(rotCfg.presetLabel[2]);
    ui->lineEdit_Label_3->setText(rotCfg.presetLabel[3]);
    ui->lineEdit_Label_4->setText(rotCfg.presetLabel[4]);
    ui->lineEdit_Label_5->setText(rotCfg.presetLabel[5]);
    ui->lineEdit_Label_6->setText(rotCfg.presetLabel[6]);
    ui->lineEdit_Label_7->setText(rotCfg.presetLabel[7]);
    ui->lineEdit_Label_8->setText(rotCfg.presetLabel[8]);

    ui->spinBox_Point_0->setValue(rotCfg.presetAz[0]);
    ui->spinBox_Point_1->setValue(rotCfg.presetAz[1]);
    ui->spinBox_Point_2->setValue(rotCfg.presetAz[2]);
    ui->spinBox_Point_3->setValue(rotCfg.presetAz[3]);
    ui->spinBox_Point_4->setValue(rotCfg.presetAz[4]);
    ui->spinBox_Point_5->setValue(rotCfg.presetAz[5]);
    ui->spinBox_Point_6->setValue(rotCfg.presetAz[6]);
    ui->spinBox_Point_7->setValue(rotCfg.presetAz[7]);
    ui->spinBox_Point_8->setValue(rotCfg.presetAz[8]);
}

void DialogPreset::on_buttonBox_accepted()
{
    QSettings configFile(QString("catrotator.ini"), QSettings::IniFormat);

    configFile.beginWriteArray("Preset");

    configFile.setArrayIndex(0);
    configFile.setValue("presetLabel", ui->lineEdit_Label_0->text());
    configFile.setValue("presetAz", ui->spinBox_Point_0->value());
    configFile.setArrayIndex(1);
    configFile.setValue("presetLabel", ui->lineEdit_Label_1->text());
    configFile.setValue("presetAz", ui->spinBox_Point_1->value());
    configFile.setArrayIndex(2);
    configFile.setValue("presetLabel", ui->lineEdit_Label_2->text());
    configFile.setValue("presetAz", ui->spinBox_Point_2->value());
    configFile.setArrayIndex(3);
    configFile.setValue("presetLabel", ui->lineEdit_Label_3->text());
    configFile.setValue("presetAz", ui->spinBox_Point_3->value());
    configFile.setArrayIndex(4);
    configFile.setValue("presetLabel", ui->lineEdit_Label_4->text());
    configFile.setValue("presetAz", ui->spinBox_Point_4->value());
    configFile.setArrayIndex(5);
    configFile.setValue("presetLabel", ui->lineEdit_Label_5->text());
    configFile.setValue("presetAz", ui->spinBox_Point_5->value());
    configFile.setArrayIndex(6);
    configFile.setValue("presetLabel", ui->lineEdit_Label_6->text());
    configFile.setValue("presetAz", ui->spinBox_Point_6->value());
    configFile.setArrayIndex(7);
    configFile.setValue("presetLabel", ui->lineEdit_Label_7->text());
    configFile.setValue("presetAz", ui->spinBox_Point_7->value());
    configFile.setArrayIndex(8);
    configFile.setValue("presetLabel", ui->lineEdit_Label_8->text());
    configFile.setValue("presetAz", ui->spinBox_Point_8->value());

    configFile.endArray();

    emit configDone();
}

void DialogPreset::on_pushButton_Default_clicked()
{
    int defaultPreset[9] = {0, 45, 90, 135, 180, 225, 270, 315, 360};

    for (int i = 0; i < 9; i++)
    {
        rotCfg.presetLabel[i]=QString::number(defaultPreset[i]);
        rotCfg.presetAz[i]=defaultPreset[i];
    }

    DialogPreset::presetInit();
}


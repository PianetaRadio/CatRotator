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


#include "rotdaemon.h"
#include "rotatordata.h"

#include <QThread>
#include <QMessageBox>
#include <QDebug>

#include <rotator.h>

ROT *my_rot;

extern rotatorConnect rotCom;
extern rotatorSettings rotGet;


RotDaemon::RotDaemon(QObject *parent) : QObject(parent)
{

}

int RotDaemon::rotConnect()
{
    int retcode;

    my_rot = rot_init(rotCom.rotModel); //Allocate rig handle

    if (!my_rot)    //Wrong Rig number
    {
        QMessageBox msgBox; //Show error MessageBox
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Rotator model error");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        return -1;   //RIG_EINVAL, Invalid parameter
    }
    else
    {
        if (rotCom.rotModel == 2)   //Rotctld
        {
            strncpy(my_rot->state.rotport.pathname, rotCom.rotPort.toLatin1(), HAMLIB_FILPATHLEN - 1);
        }
        else
        {
            strncpy(my_rot->state.rotport.pathname, rotCom.rotPort.toLatin1(), HAMLIB_FILPATHLEN - 1);
            my_rot->state.rotport.parm.serial.rate = rotCom.serialSpeed;
        }

        retcode = rot_open(my_rot);

        if (retcode != RIG_OK) return retcode;  //Rig not connected
        else    //Rig connected
        {
            return 0;
        }

     }
}

void RotDaemon::rotUpdate()
{
    //int retcode;

    rot_get_position(my_rot, &rotGet.az, &rotGet.el);

    emit resultReady();
}

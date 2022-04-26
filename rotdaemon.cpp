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

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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFileSystemWatcher>


#define RELEASE_DATE __DATE__
#define VERSION_MAJ 1
#define VERSION_MIN 5
#define VERSION_MIC 1

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void rotUpdate();   //Slot fot QTimer
    void on_rotDaemonResultReady(int rotNumber);    //Slot for rotDaemon resultReady

    void on_lineEditEnterPressed();     //Slot for enter pressed in lineEdit
    void on_lineEditEnterPressed2();
    void on_lineEditEnterPressed3();

    //void parseWSJTXStatus(QString *status);    //Read WSJT-X wsjtx_status.txt locator or call
    void parseWSJTXStatus();

private slots:
    void on_actionRotator_triggered();

    void on_actionAbout_CatRotator_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionAbout_Hamlib_triggered();

    void on_pushButton_connect_toggled(bool checked);

    void on_pushButton_go_clicked();

    void on_pushButton_park_clicked();

    void on_pushButton_stop_clicked();

    void on_actionSetup_triggered();

    void on_pushButton_go_2_clicked();

    void on_pushButton_p0_clicked();

    void on_pushButton_p1_clicked();

    void on_pushButton_p2_clicked();

    void on_pushButton_p3_clicked();

    void on_pushButton_p4_clicked();

    void on_pushButton_p5_clicked();

    void on_pushButton_p6_clicked();

    void on_pushButton_p7_clicked();

    void on_pushButton_p8_clicked();

    void on_actionCatRotator_homepage_triggered();

    void on_actionPresets_triggered();

    void on_toolButton_pathSL_toggled(bool checked);

    void on_toolButton_pathSL_2_toggled(bool checked);

    void on_pushButton_go_3_clicked();

    void on_toolButton_pathSL_3_toggled(bool checked);

    void on_toolButton_minus_clicked();

    void on_toolButton_plus_clicked();

    void on_toolButton_minus_2_clicked();

    void on_toolButton_plus_2_clicked();

    void on_toolButton_minus_3_clicked();

    void on_toolButton_plus_3_clicked();

    void on_toolButton_track_toggled(bool checked);

    void on_toolButton_track_2_toggled(bool checked);

    void on_toolButton_track_3_toggled(bool checked);

    void on_actionAbout_cty_dat_triggered();

    void on_actionAbout_DarkTheme_triggered();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    QFileSystemWatcher statusWsjtxWatch;
    QString statusWsjtx;
    QString versionCTY();  //Extract CTY.DAT version

    void guiInit();
    void guiUpdate(int rotNumber);
    void presetGo(int presetNumber);
    void presetInit();
    void setPosition(int rot, float azim, float elev);

    bool azElInput(QString value, bool lPath, double *azim, double *elev);  //Convert pointing value input, from format degree or QTH locator, into azimuth bearing angle and elevation
    bool parseCTY(QString callsign, QString *countryName, QString *country, double *lat, double *lon); //Find Country in CTY.DAT from callsign

    void parseWSJTXMoon(double *azim, double *elev);    //Read WSJT-X azel.dat tracking data
    void parseAirScout(double *azim, double *elev); //Read AirScout azel.dat tracking data

    bool bearingAngle(const char *locator1, const char *locator2, double *azim, double *dist);  //Calculate Short Path bearing angle and distance between two locators
    bool bearingAngleLP(const char *locator1, const char *locator2, double *azim, double *dist);    //Calculate Long Path bearing angle and distance between two locators

    bool checkHamlibVersion(int major, int minor, int revision);    //Check hamlib version
};

#endif // MAINWINDOW_H

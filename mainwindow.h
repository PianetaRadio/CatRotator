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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#define RELEASE_DATE __DATE__
#define VERSION_MAJ 0
#define VERSION_MIN 1
#define VERSION_MIC 0

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
    void on_rotDaemonResultReady();    //Slot for rotDaemon resultReady

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

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    void guiInit();
    void guiUpdate();
};

#endif // MAINWINDOW_H

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

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    void guiUpdate();
};

#endif // MAINWINDOW_H

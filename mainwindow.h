#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QIODevice>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>

#include <unistd.h>  // For "getuid" and "geteuid"
#include <QIntValidator>  // Added since QIntValidator was moved to it's own header

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setSettingsPath();
    void applySettings(QString settingFile, QString settingValue);
    void validateSettings();
    void displaySettings();
    void origionalFunction();
    void installTrackpointSH();
    void installTrackpointService();
    void installTrackpointTimer();
    void setPersistPaths();
    void checkMode();
    void setTestMode(int testMode);
    void checkDeviceMgr();

private slots:
    void on_button_Apply_clicked();

    void on_button_Defaults_clicked();

    void on_check_Persist_clicked();

    void on_actionToggle_Test_Mode_triggered();

    void on_check_Scrolling_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QString settingPath;
    QString trackpointSHPath;
    QString initPath;
    QString initCommand;
    QString testPath;
    QString testXorgPath;
    int testStatus;
    QString xorgLocation;
    QString xinputCommand;
    QStringList xinputArgs;
    QProcess runXinput;
    int runMode;
};

#endif // MAINWINDOW_H

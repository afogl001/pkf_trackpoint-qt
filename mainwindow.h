#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QMessageBox>

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

private slots:
    void on_button_Apply_clicked();

private:
    Ui::MainWindow *ui;
    QString settingPath;
    QString speedPath;
    QString sensativityPath;
    QString press_to_selectPath;
};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setSettingsPath();
    validateSettings();
    displaySettings();
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_button_Apply_clicked()
{
    applySettings(settingPath + "/speed", ui->edit_Speed->text());
    applySettings(settingPath + "/sensitivity", ui->edit_Sensativity->text());
    applySettings(settingPath + "/press_to_select", ui->edit_Press_To_Select->text());

    displaySettings();

    ui->edit_Speed->clear();
    ui->edit_Sensativity->clear();
    ui->edit_Press_To_Select->clear();


}

void MainWindow::setSettingsPath()
{
    if (QFile::exists("/tmp/pkf_trackpoint/speed"))  // Make directory "/tmp/pkf_trackpoint" with "speed", "sensativity", and "press_to_select" if you'd like to test
        settingPath = "/tmp/pkf_trackpoint";
    else if (QFile::exists("/sys/devices/platform/i8042/serio1/serio2/speed"))  // If trackpad exists
        settingPath = "/sys/devices/platform/i8042/serio1/serio2";
    else if (QFile::exists("/sys/devices/platform/i8042/serio1/speed"))  // if trackpad does not exist
        settingPath = "/sys/devices/platform/i8042/serio1";
    else
    {
        QMessageBox::critical(this, "Alert", "No TrackPoint detected!");
        exit(EXIT_FAILURE);
    }

    qDebug() << "Using trackpoint settings at " << settingPath;
}

void MainWindow::applySettings(QString settingFile, QString settingValue)
{
    QFile fileSet(settingFile);
    if (settingValue!=NULL) {  // Only update setting if the lineedit is not NULL
        if (fileSet.open(QIODevice::ReadWrite | QIODevice::Truncate)) {   // "QIODevice::Truncate" deletes the content of the file!
            QTextStream stream(&fileSet);
            stream << settingValue;
        }
        fileSet.close();
    }
}

void MainWindow::validateSettings()
{
    QValidator *validateSpeedSensativity = new QIntValidator(1,255,this);
    QValidator *validatePress_To_Select = new QIntValidator(0,1,this);
    ui->edit_Speed->setValidator(validateSpeedSensativity);
    ui->edit_Sensativity->setValidator(validateSpeedSensativity);
    ui->edit_Press_To_Select->setValidator(validatePress_To_Select);
}

void MainWindow::displaySettings()
{
    QFile speedFile(settingPath + "/speed");
    if(!speedFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", speedFile.errorString());
    }
    ui->label_Speed->setText(speedFile.readAll());
    speedFile.close();

    QFile sensativityFile(settingPath + "/sensitivity");
    if(!sensativityFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", sensativityFile.errorString());
    }
    ui->label_Sensitivity->setText(sensativityFile.readAll());
    sensativityFile.close();

    QFile press_to_selectFile(settingPath + "/press_to_select");
    if(!press_to_selectFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", press_to_selectFile.errorString());
    }
    ui->label_PressToSelect->setText(press_to_selectFile.readAll());
    press_to_selectFile.close();
}

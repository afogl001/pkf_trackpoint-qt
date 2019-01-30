#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setSettingsPath();
    setPersistPaths();
    checkUser();
    validateSettings();
    displaySettings();

    if (QFile::exists(trackpointSHPath + "/trackpoint.sh"))
        ui->check_Persist->setChecked(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::checkUser()
{
        auto user = getuid();
        auto effectiveUser = geteuid();
        qDebug() << "Actual User = " << user;
        qDebug() << "Effective User = " << effectiveUser;

        if (effectiveUser == 0)
            initCommand = "systemctl";
        else {
            initCommand = "echo systemctl";
            ui->label_Info->setText("Running in \"ReadOnly\" or \"Test\" Mode:  Run with \"sudo\" or as root to change to TrackPoint settings");
        }
}

void MainWindow::on_button_Apply_clicked()
{
    applySettings(settingPath + "/speed", ui->edit_Speed->text());
    applySettings(settingPath + "/sensitivity", ui->edit_Sensativity->text());
    applySettings(settingPath + "/press_to_select", ui->edit_Press_To_Select->text());

    displaySettings();

    if (ui->check_Persist->isChecked())
        installTrackpointSH();

    ui->edit_Speed->clear();
    ui->edit_Sensativity->clear();
    ui->edit_Press_To_Select->clear();
}

void MainWindow::setSettingsPath()
{
    if (QFile::exists("/tmp/pkf_trackpoint/speed") && geteuid()!=0)  // Make directory "/tmp/pkf_trackpoint" with "speed", "sensitivity", and "press_to_select" if you'd like to test
        settingPath = "/tmp/pkf_trackpoint";
    else if (QFile::exists("/sys/devices/platform/i8042/serio1/serio2/speed"))  // If trackpad exists
        settingPath = "/sys/devices/platform/i8042/serio1/serio2";
    else if (QFile::exists("/sys/devices/platform/i8042/serio1/speed"))  // If trackpad does not exist
        settingPath = "/sys/devices/platform/i8042/serio1";
    else if (QFile::exists("/sys/devices/rmi4-00/rmi4-00.fn03/serio2/speed"))  // If disto uses less common device directory (suc has KDE Neon)
        settingPath = "/sys/devices/rmi4-00/rmi4-00.fn03/serio2";
    else
    {
        QMessageBox::critical(this, "Alert", "No TrackPoint detected!");
        exit(EXIT_FAILURE);
    }

    qDebug() << "Using trackpoint settings at " << settingPath;
}

void MainWindow::setPersistPaths()
{
    if (QFile::exists("/tmp/pkf_trackpoint/speed")) {  // Make directory "/tmp/pkf_trackpoint" with "speed" if you'd like to test
        trackpointSHPath = "/tmp/pkf_trackpoint";
        initPath = "/tmp/pkf_trackpoint";
    }
    else if (QFile::exists("/etc/systemd/system")) { // If trackpad exists
        trackpointSHPath = "/usr/bin";
        initPath = "/etc/systemd/system";
    }
    else {
        trackpointSHPath = "N/A";
        initPath = "N/A";
    }

     qDebug() << "Will place trackpoint.sh in " << trackpointSHPath;
     qDebug() << "Will place initalization files in" << initPath;
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
        QMessageBox::warning(0, "Warning", speedFile.errorString());
    }
    ui->label_Speed->setText(speedFile.readAll());
    speedFile.close();

    QFile sensativityFile(settingPath + "/sensitivity");
    if(!sensativityFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(0, "Warning", sensativityFile.errorString());
    }
    ui->label_Sensitivity->setText(sensativityFile.readAll());
    sensativityFile.close();

    QFile press_to_selectFile(settingPath + "/press_to_select");
    if(!press_to_selectFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(0, "Warning", press_to_selectFile.errorString());
    }
    ui->label_PressToSelect->setText(press_to_selectFile.readAll());
    press_to_selectFile.close();
}

void MainWindow::on_button_Defaults_clicked()
{
    applySettings(settingPath + "/speed", "97");
    applySettings(settingPath + "/sensitivity", "128");
    applySettings(settingPath + "/press_to_select", "0");

    displaySettings();

    if (ui->check_Persist->isChecked())
        installTrackpointSH();
}

void MainWindow::on_check_Persist_clicked()
{
    if (ui->check_Persist->isChecked()){
        if (trackpointSHPath=="N/A") {
            QMessageBox::critical(this, "Alert", "Initalization system not detected!");
            ui->check_Persist->setChecked(0);
        }
         else {
            installTrackpointSH();
            installTrackpointService();
            installTrackpointTimer();

            QProcess::execute(initCommand + " daemon-reload");
            QProcess::execute(initCommand + " start trackpoint");
            QProcess::execute(initCommand + " enable trackpoint.timer");
         }
    }

    if (!ui->check_Persist->isChecked()){
        if (trackpointSHPath=="N/A")
            qDebug() << "No initalizatoin system detected so no change made by unchecking";
        else {
            QProcess::execute(initCommand + " disable trackpoint.timer");
            QProcess::execute(initCommand + " stop trackpoint");

            QFile trackpointSH(trackpointSHPath + "/trackpoint.sh");
            trackpointSH.remove();
            QFile trackpointService(trackpointSHPath + "/trackpoint.service");
            trackpointService.remove();
            QFile trackpointTimer(trackpointSHPath + "/trackpoint.timer");
            trackpointTimer.remove();

            QProcess::execute(initCommand + " daemon-reload");
        }
    }
}

void MainWindow::installTrackpointSH()
{
    QFile trackpointSH(trackpointSHPath + "/trackpoint.sh");
    trackpointSH.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream stream(&trackpointSH);
    stream << "#!/bin/bash\n\n";
    stream << "## Determine and set correct path for trackpoint\n";
    stream << "if [ -f /sys/devices/platform/i8042/serio1/serio2/sensitivity ];\n";
    stream << "then\n";
    stream << "  vTrackpointPath=/sys/devices/platform/i8042/serio1/serio2\n";
    stream << "elif [ -f /sys/devices/platform/i8042/serio1/sensitivity ];\n";
    stream << "then\n";
    stream << "  vTrackpointPath=/sys/devices/platform/i8042/serio1\n";
    stream << "elif [ -f /sys/devices/rmi4-00/rmi4-00.fn03/serio2/sensitivity ];\n";
    stream << "then\n";
    stream << "  vTrackpointPath=/sys/devices/rmi4-00/rmi4-00.fn03/serio2\n";
    stream << "else\n";
    stream << "  echo \"Trackpoint not detected\"\n";
    stream << "  exit 200\n";
    stream << "fi\n\n";
    stream << "## Set trackpoint setting variable values\n";
    stream << "vSensitivity=" << ui->label_Sensitivity->text() << "\n";
    stream << "vSpeed=" << ui->label_Speed->text() << "\n";
    stream << "vPress_to_Select=" << ui->label_PressToSelect->text() << "\n\n";
    stream << "### Apply settings based on option \"start\" or \"stop\" being passed\n";
    stream << "case $1 in\n";
    stream << "  [Ss]tart )/n";
    stream << "    echo -n \"$vSensitivity > $vTrackpointPath/sensativity\n";
    stream << "    echo -n \"$vSpeed > $vTrackpointPath/speed\n";
    stream << "    echo -n \"$vPress_to_Select > $vTrackpointPath/press_to_select\n";
    stream << "  ;;\n\n";
    stream << "  [Ss]top )\n";
    stream << "    echo 128 > $vTrackpointPath/sensitivity\n";
    stream << "    echo 97 > $vTrackpointPath/speed\n";
    stream << "    echo 0 > $vTrackpointPath/press_to_select";
    stream << "  ;;\n\n";
    stream << "  * )\n";
    stream << "    echo \"Usage: {start|stop}\n";
    stream << "esac";
    trackpointSH.close();
}

void MainWindow::installTrackpointService()
{
    QFile trackpointService(initPath + "/trackpoint.service");
    trackpointService.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream stream(&trackpointService);
    stream << "[Unit]\n";
    stream << "Description=Trackpoint Configuration for systemd\n";
    stream << "## Only using \"sensitivity\" file as also using \"speed\" and \"press_to_select\" would be redundant\n";
    stream << "ConditionPathExists=|/sys/devices/platform/i8042/serio1/serio2/sensitivity\n";
    stream << "ConditionPathExists=|/sys/devices/platform/i8042/serio1/sensitivity\n";
    stream << "ConditionPathExists=|/sys/devices/rmi4-00/rmi4-00.fn03/serio2/sensitivity\n\n";
    stream << "[Service]\n";
    stream << "Type=oneshot\n";
    stream << "RemainAfterExit=yes\n";
    stream << "ExecStart=/usr/bin/trackpoint.sh start\n";
    stream << "ExecStop=/usr/bin/trackpoint.sh stop\n\n";
    stream << "[Install]\n";
    stream << "WantedBy=graphical.target";
    trackpointService.close();
}

void MainWindow::installTrackpointTimer()
{
    QFile trackpointTimer(initPath + "/trackpoint.timer");
    trackpointTimer.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream stream(&trackpointTimer);
    stream << "[Unit]\n";
    stream << "Description=Activates trackpoint after boot\n\n";
    stream << "[Timer]\n";
    stream << "OnActiveSec=15\n";
    stream << "Persistent=true\n\n";
    stream << "[Install]\n";
    stream << "WantedBy=timers.target";
    trackpointTimer.close();
}

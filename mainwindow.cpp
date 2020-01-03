#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    checkDeviceMgr();
    setSettingsPath();
    setPersistPaths();
    checkMode();
    validateSettings();
    displaySettings();
}

MainWindow::~MainWindow()
{
    delete ui;
    setTestMode(3);
}

void MainWindow::checkDeviceMgr()
{
    QProcess eventDevice;
    QString exec = "xinput";
    QStringList params;
     params << "list-props" << "TPPS/2 IBM TrackPoint";

    eventDevice.start(exec, params);
    eventDevice.waitForFinished(); // sets current thread to sleep and waits for process to end
    QString output(eventDevice.readAllStandardOutput());
    qDebug() << "Using Evdev: " << output.contains("Evdev");
    if (output.contains("Evdev")) {
       ui->check_Scrolling->setVisible(true);
       // Determine and set correct path for X11 xorg.conf.d
       QStringList xOrgDirList;  // Declare list for directories to search
       xOrgDirList << "/tmp" <<  "/usr" << "/etc";  // Specify directories to search
       bool fileFound = false;

       foreach (QString element, xOrgDirList) {
        QDirIterator xorg_check(element, QStringList() << "xorg.conf.d", QDir::NoFilter, QDirIterator::Subdirectories);
        while (xorg_check.hasNext() && fileFound == false) {  // Search recursivly for "xorg.conf.d"
            qDebug() << "Checking for xorg.conf.d, found: " << xorg_check.next();  // For some reason, this is needed or eles the infinite loop
            xorgLocation = xorg_check.fileInfo().absoluteFilePath();  // Assign "xorg.conf.d" directory to variable
            qDebug() << "Using xorg.conf.d directory: " << xorgLocation;

            QFileInfo fileInfoXorg(xorgLocation+"/90-trackpoint.conf");  // Set object for "90-trackpoint.conf" file
            qDebug() << "Scrolling file: " << fileInfoXorg.exists();
            if (fileInfoXorg.exists()) {  // If "90-trackpoint.conf"...
                ui->check_Scrolling->setChecked(1);
                fileFound = true;
           }
            else ui->check_Scrolling->setChecked(0);
        }
       }
    }
}

void MainWindow::checkMode()
{
        auto effectiveUser = geteuid();

        if (effectiveUser == 0 && testStatus!=1) {
            initCommand = "systemctl";
            ui->label_Info->setText("");
            runMode=0;
        }
        else if (testStatus==1) {
            initCommand = "echo systemctl";
            ui->label_Info->setText("Running in \"Test\" Mode");
            runMode=1;
        }
        else {
            initCommand = "echo systemctl";
            ui->label_Info->setText("Running in \"ReadOnly\" Mode:  Run with \"sudo\" or as root to change to TrackPoint settings");
            runMode=2;
        }
        qDebug() << "Effective User = " << effectiveUser;
        qDebug() << "initCommand = " << initCommand;
}

void MainWindow::setTestMode(int testMode)
{
    testStatus=testMode;
    qDebug() << "Setting \"Test Mode\" to" << testMode;
    testPath = "/tmp/pkf_trackpoint";
    QDir dir(testPath);
    if (testMode==1) {
        qDebug() << "Test path = " << testPath;
        dir.mkdir(testPath);
        QProcess::execute("mkdir " + testPath + "/xorg.conf.d");
        QProcess::execute("touch " + testPath + "/xorg.conf.d/90-trackpoint.conf");
        xinputCommand = "echo xinput set-prop \"TPPS/2 IBM TrackPoint\"";
        qDebug() << "Test xorg command = " << xinputCommand;

        QFile testSpeed(testPath + "/speed");
        testSpeed.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QTextStream streamSpeed(&testSpeed);
        streamSpeed << "Test";
        testSpeed.close();

        QFile testSensitivity(testPath + "/sensitivity");
        testSensitivity.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QTextStream streamSensitivity(&testSensitivity);
        streamSensitivity << "Test";
        testSensitivity.close();

        QFile testPress_to_select(testPath + "/press_to_select");
        testPress_to_select.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QTextStream streamPress_to_select(&testPress_to_select);
        streamPress_to_select << "Test";
        testPress_to_select.close();

        setSettingsPath();
        setPersistPaths();
        displaySettings();
        checkMode();
        checkDeviceMgr();
    }
    else if (testMode==0) {
        qDebug() << "Removong " << testPath;
        dir.removeRecursively();
        xinputCommand = "xinput set-prop \"TPPS/2 IBM TrackPoint\"";
        qDebug() << "Xorg command = " << xinputCommand;

        setSettingsPath();
        setPersistPaths();
        displaySettings();
        checkMode();
        checkDeviceMgr();
    }
    else if (testMode==3) {
        qDebug() << "Removong " << testPath << " if it exists";
        dir.removeRecursively();
    }
    else
        qDebug() << "Use options 1-3 for setting Test Mode";
}

void MainWindow::on_button_Apply_clicked()
{
    applySettings(settingPath + "/speed", ui->edit_Speed->text());
    applySettings(settingPath + "/sensitivity", ui->edit_Sensitivity->text());
    applySettings(settingPath + "/press_to_select", ui->edit_Press_To_Select->text());

    displaySettings();

    if (ui->check_Persist->isChecked())
        installTrackpointSH();

    ui->edit_Speed->clear();
    ui->edit_Sensitivity->clear();
    ui->edit_Press_To_Select->clear();
}

void MainWindow::setSettingsPath()
{
  QStringList dirList;  // Declare list for directories to search
  dirList << "/sys/devices/" << "/tmp/";  // Specify directories to search

  foreach (QString element, dirList) {
   QDirIterator file_check(element, QStringList() << "press_to_select", QDir::NoFilter, QDirIterator::Subdirectories);
   while (file_check.hasNext()) {  // Search recursivly for "press_to_select"
       qDebug() << "Checking for TrackPoint settings, found:  " << file_check.next();  // For some reason, this is needed or eles the infinite loop
       QString fileLocation = file_check.fileInfo().path();  // Assign directory containing "press_to_selectd" to variable
       QFileInfo fileInfoSpeed(fileLocation+"/speed");  // Set object for "speed" file
       QFileInfo fileInfoSensitivity(fileLocation+"/sensitivity");  // Set object for "sensitivity" file
       if (fileInfoSpeed.exists()) {  // If "speed" file exist...
          if (fileInfoSensitivity.exists())  { // and "sensitivity" file exist...
              settingPath = fileLocation;  // set the discovered path to "settingPath"
              qDebug() << "Using TrackPoint settings at: " << settingPath;
          } else {
              QMessageBox::warning(this, "Alert", "No TrackPoint detected!");
              setTestMode(1);
              }
       }

   }
  }
}

void MainWindow::setPersistPaths()
{
    if (QFile::exists("/tmp/pkf_trackpoint/speed")) {  // Make directory "/tmp/pkf_trackpoint" with "speed", "sensitivity", and "press_to_select" if you'd like to test
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

    if (QFile::exists(trackpointSHPath + "/trackpoint.sh"))
        ui->check_Persist->setChecked(1);
    else
        ui->check_Persist->setChecked(0);

     qDebug() << "Will place trackpoint.sh in " << trackpointSHPath;
     qDebug() << "Will place initalization files in" << initPath;
     qDebug() << "Persist check-box set to " << ui->check_Persist->checkState();
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
    QValidator *validateSpeedSensitivity = new QIntValidator(1,255,this);
    QValidator *validatePress_To_Select = new QIntValidator(0,1,this);
    ui->edit_Speed->setValidator(validateSpeedSensitivity);
    ui->edit_Sensitivity->setValidator(validateSpeedSensitivity);
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

    QFile sensitivityFile(settingPath + "/sensitivity");
    if(!sensitivityFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(0, "Warning", sensitivityFile.errorString());
    }
    ui->label_Sensitivity->setText(sensitivityFile.readAll());
    sensitivityFile.close();

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
            QMessageBox::critical(this, "Alert", "Initialization system not detected!");
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
            qDebug() << "No initalization system detected so no change made by unchecking";
        else {
            QProcess::execute(initCommand + " disable trackpoint.timer");
            QProcess::execute(initCommand + " stop trackpoint");

            QFile trackpointSH(trackpointSHPath + "/trackpoint.sh");
            trackpointSH.remove();
            QFile trackpointService(initPath + "/trackpoint.service");
            trackpointService.remove();
            QFile trackpointTimer(initPath + "/trackpoint.timer");
            trackpointTimer.remove();

            QProcess::execute(initCommand + " daemon-reload");

            // Reset TrackPoint settings to "current" values (stopping trackpoint.sh sets values to default)
            applySettings(settingPath + "/speed", ui->label_Speed->text());
            applySettings(settingPath + "/sensitivity", ui->label_Sensitivity->text());
            applySettings(settingPath + "/press_to_select", ui->label_PressToSelect->text());
        }
    }
    displaySettings();
}

void MainWindow::on_actionToggle_Test_Mode_triggered()
{
    if (QFile::exists(testPath + "/speed"))
        setTestMode(0);
    else
        setTestMode(1);
}

void MainWindow::installTrackpointSH()
{
    QFile trackpointSH(trackpointSHPath + "/trackpoint.sh");
    trackpointSH.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream stream(&trackpointSH);
    stream << "#!/bin/bash\n\n";
    stream << "## Determine and set correct path for trackpoint\n";
    stream << "if [ -f " << settingPath << "/speed ];\n";
    stream << "then\n";
    stream << " vTrackpointPath=" << settingPath << "\n";
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
    stream << "  [Ss]tart )\n";
    stream << "    echo -n $vSensitivity > $vTrackpointPath/sensitivity\n";
    stream << "    echo -n $vSpeed > $vTrackpointPath/speed\n";
    stream << "    echo -n $vPress_to_Select > $vTrackpointPath/press_to_select\n";
    stream << "  ;;\n\n";
    stream << "  [Ss]top )\n";
    stream << "    echo 128 > $vTrackpointPath/sensitivity\n";
    stream << "    echo 97 > $vTrackpointPath/speed\n";
    stream << "    echo 0 > $vTrackpointPath/press_to_select\n";
    stream << "  ;;\n\n";
    stream << "  * )\n";
    stream << "    echo \"Usage: {start|stop}\"\n";
    stream << "esac";
    trackpointSH.close();
    trackpointSH.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);
}

void MainWindow::installTrackpointService()
{
    QFile trackpointService(initPath + "/trackpoint.service");
    trackpointService.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream stream(&trackpointService);
    stream << "[Unit]\n";
    stream << "Description=Trackpoint Configuration for systemd\n";
    stream << "## Only using \"speed\" file as also using \"sensitivity\" and \"press_to_select\" would be redundant\n";
    stream << "ConditionPathExists=" << settingPath << "/speed\n\n";
    stream << "[Service]\n";
    stream << "Type=oneshot\n";
    stream << "RemainAfterExit=yes\n";
    stream << "ExecStart=" << trackpointSHPath << "/trackpoint.sh start\n";
    stream << "ExecStop=" << trackpointSHPath << "/trackpoint.sh stop\n\n";
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

void MainWindow::on_check_Scrolling_stateChanged(int arg1)
{
    QFile trackpointEvdevConf(xorgLocation + "/90-trackpoint.conf");
    if (ui->check_Scrolling->isChecked()) {
        qDebug() << "check_Scrolling: Checked";
        QProcess::execute(xinputCommand + " \"Evdev Wheel Emulation\" 1");
        QProcess::execute(xinputCommand + " \"Evdev Wheel Emulation Button\" 2");
        QProcess::execute(xinputCommand + " \"Evdev Wheel Emulation Axes\" 6 7 4 5");

        if (runMode < 2) {
            trackpointEvdevConf.open(QIODevice::WriteOnly | QIODevice::Truncate);
            QTextStream stream(&trackpointEvdevConf);
            stream << "Section InputClass\n";
            stream << "\tIdentifier\t\"trackpoint catchall\"\n";
            stream << "\tMatchIsPointer\t\"true\"\n";
            stream << "\tMatchProduct\t\"TrackPoint|DualPoint Stick\"\n";
            stream << "\tMatchDevicePath\t\"/dev/input/event*\"\n";
            stream << "\tOption\t\"Emulate3Buttons\"\t\"true\"\n";
            stream << "\tOption\t\"EmulateWheel\"\t\"1\"\n";
            stream << "\tOption\t\"EmulateWheelButton\"\t\"2\"\n";
            stream << "\tOption\t\"XAxisMapping\"\t\"6 7\"\n";
            stream << "\tOption\t\"YAxisMapping\"\t\"4 5\"\n";
            stream << "EndSection";
            trackpointEvdevConf.close();
        }
    }
    else if (!ui->check_Scrolling->isChecked()) {
        qDebug() << "check_Scroling: Un-checked";
        QProcess::execute(xinputCommand + " \"Evdev Wheel Emulation\" 0");
        QProcess::execute(xinputCommand + " \"Evdev Wheel Emulation Button\" 3");
        QProcess::execute(xinputCommand + " \"Evdev Wheel Emulation Axes\" 0 0 4 5");

        if (runMode < 2) {
            trackpointEvdevConf.remove();
        }
    }
}

/* This file is part of cuteAndroidBackup.

cuteAndroidBackup is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

cuteAndroidBackup is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with cuteAndroidBackup.  If not, see <http://www.gnu.org/licenses/>. */

#include "mainwindow.h"
#include "ui_mainwindow.h"

const QString MainWindow::BACKUP_FILE_EXTENSION = "ab";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    proc(new QProcess(this))
{
    ui->setupUi(this);

    connect(proc, SIGNAL(started()),this, SLOT(procStarted()));
    connect(proc, SIGNAL(error(QProcess::ProcessError)),this, SLOT(procError(QProcess::ProcessError)));
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(procExited(int,QProcess::ExitStatus)));
    connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(progStandardOutput()));
    connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(progStandardError()));

    setDefaultConsoleColor();

    state = IDLE;

    QString adb_path = "";
    QString backup_path = "";
    QFileInfo adb_file;
    QSettings settings("fadvisor", "cuteAndroidBackup");
    adb_path = settings.value("adb_path").toString();
    backup_path = settings.value("backup_path").toString();
    ui->txtBackupPath->setText(backup_path);

    getRestoreList();

    if (adb_path.isEmpty()){
#ifdef Q_OS_LINUX
        QProcess process;
        process.start("which adb");
        process.waitForFinished(-1); // wait until finished

        adb_path = process.readAllStandardOutput();
        process.terminate();

        if (!adb_path.isEmpty()) {
            adb_path.remove(QRegExp("[\\n\\t\\r]"));
        }
#endif
        if (adb_path.isEmpty()){
            // Check if "adb" exist in the same folder with this application
            adb_file.setFile("./adb");
        } else {
            adb_file.setFile(adb_path);
        }
    } else {
        adb_file.setFile(adb_path);
    }

    // check if file exists and if yes: Is it really a file and not directory?
    if (adb_file.exists() && adb_file.isFile()) {
        ui->lineEdit_adb->setText(adb_file.canonicalFilePath());

        QString program = ui->lineEdit_adb->text();
        QStringList arguments;
        arguments.append("devices");
        arguments.append("-l");
        proc->start(program, arguments);
        state = DEVICES;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setDefaultConsoleColor(){
    // Set background color to black, and text to white
    ui->textEdit->setStyleSheet("QTextEdit { background-color: rgb(0, 0, 0) ;	color: rgb(255, 255, 255);}");
    ui->textEdit->setTextColor(Qt::white);
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::procStarted()
{
//    ui->textEdit->append("Started");
}

void MainWindow::procError(QProcess::ProcessError procError)
{
    ui->textEdit->setTextColor(Qt::red);
    ui->textEdit->append(tr("Error!!!"));
    ui->textEdit->append(proc->errorString());

    switch (procError) {
      case QProcess::FailedToStart:
        ui->textEdit->append(tr("Failed to start"));
        break;
      case QProcess::Crashed:
        ui->textEdit->append(tr("Crashed"));
        break;
    case QProcess::Timedout:
        ui->textEdit->append(tr("Timedout"));
        break;
    case QProcess::UnknownError:
        ui->textEdit->append(tr("Unknown Error"));
      default:
        ui->textEdit->append(tr("REALLY! Unknown Error"));
      }

    setDefaultConsoleColor();
}

void MainWindow::procExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    ui->textEdit->append("Done.");
    ui->textEdit->append(QString::number(exitCode));

//    if ( myProcess->exitStatus() == 0)
//    {
//    qDebug () << "Program ran successfully";
//    }
//    if ( myProcess->exitStatus() == 2)
//    {
//    qDebug () << "Customized message";
//    }
//    if ( myProcess->exitStatus() == 3)
//    {
//    qDebug () << "Another text warning message";
//    }

    switch (state){
    case DEVICES:
    {
        getAppsList();
        break;
    }
    case APPS :
    {
        QStringList items;
        AppInfo appInfo;
        // Remove carriage returns as they show up in some phones.
        buffer.replace("\\r", "");
        items = buffer.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
        buffer.clear();

        ui->listApps->clear();

        for (int i=0; i < items.size() ; i++)
        {
            QString str = items.value(i);
            // Remove "package:" from the string
            str.remove(0,8);
            QStringList parts = str.split("=", QString::KeepEmptyParts);
            appInfo.id = i;
            appInfo.fancyname = "";
            appInfo.realname = parts.value(1);
            appInfo.path = parts.value(0);

            appList.append(appInfo);

            ui->listApps->addItem(parts.value(1));
        }
        state = IDLE;
        break;
    }
    case BACKUP:
    {
        // TODO: Check the backup file size, if file is too small ( > 1K )then it means the application doesn't allow backups
        //          so maybe we need to notify the user about that and delete the small useless file

        // Remove the package from the list
        selectionList.removeFirst();

        doBackup();
        break;
    }   
    case RESTORE:
    {
        // Remove the package from the list
        selectionList.removeFirst();

        doRestore();
        break;
    }
    }

}

void MainWindow::progStandardOutput()
{
    QString tmp = proc->readAllStandardOutput();
    ui->textEdit->append(tmp);

    if (state == APPS) {
        buffer.append(tmp);
    } else if (state == BACKUP) {

    }
}

void MainWindow::progStandardError()
{
    QString tmp = proc->readAllStandardError();
    ui->textEdit->append(tmp);
}

void MainWindow::on_btnADBpath_clicked()
{
    QString adb_path = QFileDialog::getOpenFileName(this, tr("Select the adb file"),"./","adb (adb)");

    if (!adb_path.isNull()) {
        ui->lineEdit_adb->setText(adb_path);
        QSettings settings("fadvisor", "cuteAndroidBackup");
        settings.setValue("adb_path", adb_path);
    }
}

void MainWindow::on_btnBackupPath_clicked()
{
    QString backup_path = QFileDialog::getExistingDirectory(this, tr("Select the backup and restore folder"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!backup_path.isNull()) {
        ui->txtBackupPath->setText(backup_path);
        QSettings settings("fadvisor", "cuteAndroidBackup");
        settings.setValue("backup_path", backup_path);
    }
}

void MainWindow::on_checkBox_details_stateChanged(int status)
{
    // status 0 => Hide, 2 => Show
    if (status == 0) {
        ui->textEdit->setVisible(false);
        // Using this to make it resize when hiding the console (not working as I want it so it is disabled for now)
//         QMetaObject::invokeMethod(this, "adjustSize", Qt::QueuedConnection);

    } else {
        ui->textEdit->setVisible(true);
    }
}

void MainWindow::on_btnAppsRefresh_clicked()
{
    getAppsList();
}

void MainWindow::getAppsList()
{
    QString program = ui->lineEdit_adb->text();
    QStringList arguments;
    arguments.append("shell");
    arguments.append("pm");
    arguments.append("list");
    arguments.append("packages");
    arguments.append("-f");
    proc->start(program, arguments);
    state = APPS;
}

void MainWindow::on_btnRestoreRefresh_clicked()
{
    getRestoreList();
}

void MainWindow::getRestoreList()
{
    QString backup_path = ui->txtBackupPath->text();

    if (backup_path.isEmpty()) {
        ui->textEdit->setTextColor(Qt::red);
        ui->textEdit->append(tr("ERROR: Select the backup path first."));
        setDefaultConsoleColor();
    } else {
        ui->listRestoreApps->clear();

        QDir dirBackup(backup_path);
        QStringList filter("*." + BACKUP_FILE_EXTENSION);
        QStringList filesList = dirBackup.entryList(QStringList(filter), QDir::Files);

        for (int i=0; i < filesList.size() ; i++)
        {
            QString file = filesList.value(i);

            // Remove the file extension BACKUP_FILE_EXTENSION, and add one for the dot
            file.chop(BACKUP_FILE_EXTENSION.length() + 1);
            ui->listRestoreApps->addItem(file);
        }
    }

}

void MainWindow::on_btnBackup_clicked()
{
    QString backup_path = ui->txtBackupPath->text();

    if (backup_path.isEmpty()) {
        ui->textEdit->setTextColor(Qt::red);
        ui->textEdit->append(tr("ERROR: Select the backup path first."));
        setDefaultConsoleColor();
    } else {
        selectionList = ui->listApps->selectedItems();
        if (selectionList.size() < 1) {
            ui->textEdit->setTextColor(Qt::red);
            ui->textEdit->append(tr("ERROR: Select the packages first."));
            setDefaultConsoleColor();
        } else {
            doBackup();
        }
    }
}

void MainWindow::doBackup()
{
    // If list size > 0 then we have items to backup still
    if (selectionList.size() > 0)
    {
        state = BACKUP;
        QString package = selectionList.first()->text();
        QString backup_path = ui->txtBackupPath->text();
        QString program = ui->lineEdit_adb->text();
        QStringList arguments;
        QString backupType = "data";
        arguments.append("backup");
        if (ui->rbtnBackupDataApp->isChecked()) {
            arguments.append("-apk");
            backupType = "data_apk";
        } else if (ui->rbtnBackupDataAppExp->isChecked()) {
            arguments.append("-apk");
            arguments.append("-obb");
            backupType = "data_apk_exp";
        }
        arguments.append("-f");
        arguments.append(backup_path + "/" + package + "-" + backupType + "." + BACKUP_FILE_EXTENSION);
        arguments.append(package);

        proc->start(program, arguments);

        ui->textEdit->append(tr("* Backing up (") + package + tr(")"));
        ui->textEdit->setTextColor(Qt::green);
        ui->textEdit->append(tr("Now unlock your device and confirm the backup operation."));
        setDefaultConsoleColor();
    } else {
        state = IDLE;
    }
}

void MainWindow::checkBackup()
{
    QString package = selectionList.first()->text();
    QString backup_path = ui->txtBackupPath->text();
    QString backupType = "data";
    if (ui->rbtnBackupDataApp->isChecked()) {
        backupType = "data_apk";
    } else if (ui->rbtnBackupDataAppExp->isChecked()) {
        backupType = "data_apk_exp";
    }

    QFileInfo backupFile((backup_path + "/" + package + "-" + backupType + "." + BACKUP_FILE_EXTENSION));
    if (backupFile.size() < MINIMUM_FILE_SIZE){

    }
}

void MainWindow::on_btnRestore_clicked()
{
    QString backup_path = ui->txtBackupPath->text();

    if (backup_path.isEmpty()) {
        ui->textEdit->setTextColor(Qt::red);
        ui->textEdit->append(tr("ERROR: Select the backup path first."));
        setDefaultConsoleColor();
    } else {
        selectionList = ui->listRestoreApps->selectedItems();
        if (selectionList.size() < 1) {
            ui->textEdit->setTextColor(Qt::red);
            ui->textEdit->append(tr("ERROR: Select the packages first."));
            setDefaultConsoleColor();
        } else {
            doRestore();
        }
    }
}

void MainWindow::doRestore()
{
    // If list size > 0 then we have items to backup still
    if (selectionList.size() > 0)
    {
        state = RESTORE;
        QString file = selectionList.first()->text();
        QString backup_path = ui->txtBackupPath->text();

        // Remove the file from the list
        selectionList.removeFirst();
        QString program = ui->lineEdit_adb->text();
        QStringList arguments;
        arguments.append("restore");
        arguments.append(backup_path + "/" + file + "." + BACKUP_FILE_EXTENSION);
        proc->start(program, arguments);

        ui->textEdit->append(tr("* Restoring (") + file + tr(")"));
    } else {
        state = IDLE;
    }
}


void MainWindow::on_actionAbout_triggered()
{
    QString myTitle = "cuteAndroidBackup";
    QString myBody = tr("<b>cuteAndroidBackup by fadvisor.net</b><br><br>"
                        "A graphical interface for 'adb'<br>"
                        "Source code: <a href='https://github.com/fduraibi/cuteAndroidBackup'>https://github.com/fduraibi/cuteAndroidBackup</a><br>"
                        "Developed by: Fahad Alduraibi<br>"
                        "v1.0");
    QMessageBox::about(this, myTitle, myBody);
}



/*
 qWarning("list size=%d\n", items.size());

 QString::number(i)

*/

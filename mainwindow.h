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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QStandardItemModel>
#include <QListWidgetItem>

// Application states
#define IDLE    0
#define DEVICES 1
#define APPS    2
#define BACKUP  3

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_INVOKABLE void adjustSize() { QWidget::adjustSize(); }

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void procStarted();
    void procError(QProcess::ProcessError procError);
    void procExited(int exitCode, QProcess::ExitStatus exitStatus);
    void progStandardOutput();
    void progStandardError();

    void on_toolBtn_adb_clicked();

    void on_checkBox_details_stateChanged(int status);

    void on_btnBackup_clicked();
    void on_btnRestore_clicked();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    QProcess * proc;

    void setDefaultConsoleColor();
    void doBackup();

    int state;
    QString buffer;
    QList<QListWidgetItem *> selectionList;

    struct AppInfo {
      int id;
      QString fancyname;
      QString realname;
      QString path;
    } ;
    QList<AppInfo> appList;
};

#endif // MAINWINDOW_H

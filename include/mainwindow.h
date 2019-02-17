#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialog>
#include <QLabel>
#include <QtNetwork/QNetworkReply>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void userStartStationChanged();
    void userEndStationChanged();
protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void about();
    void changeTrain();
    void changeStation();
    void cancelTicket();
    void editConfiguration();
    void setUpTableView();
    void login();
    void refreshVarificationImage();
    void submitLoginRequest();
    void queryTicket();
private:
    void createUiComponent();
    void createStatusBars();
    void readSettings();
    void writeSettings();
    void handleReply();
    void proccessVarificationResponse(QNetworkReply *reply);
    void proccessQueryTicketResponse(QNetworkReply *reply);
private:
    Ui::MainWindow *ui;
    QTableView *tableView;
    QStandardItemModel *model;
    QDialog *loginDialog;
    QDialog *SettingDialog;
};

#endif // MAINWINDOW_H

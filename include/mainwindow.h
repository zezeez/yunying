#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialog>
#include <QLabel>
#include <QDate>
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
    void setRemainTicketColor(QString &remain, QStandardItem *item);

public slots:
    void userStartStationChanged();
    void userEndStationChanged();
    void userTourDateChanged(const QDate &date);
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
    void swapStation();
private:
    void createUiComponent();
    void createStatusBars();
    void readSettings();
    void writeSettings();
    void handleReply();
    void proccessVarificationResponse(QNetworkReply *reply);
    void proccessQueryTicketResponse(QNetworkReply *reply);
    void proccessStationNameTxtResponse(QNetworkReply *reply);
private:
    Ui::MainWindow *ui;
    QTableView *tableView;
    QStandardItemModel *model;
    QDialog *loginDialog;
    QDialog *SettingDialog;
};

#endif // MAINWINDOW_H

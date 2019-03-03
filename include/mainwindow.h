#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialog>
#include <QLabel>
#include <QDate>
#include <QtNetwork/QNetworkReply>
#include <QTextBrowser>

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
    void closeEvent(QCloseEvent *event);

private slots:
    void about();
    void changeTrain();
    void changeStation();
    void cancelTicket();
    void editConfiguration();
    void setUpTableView();
    void showLoginDialog();
    void refreshVarificationImage();
    void submitLoginRequest();
    void queryTicket();
    void swapStation();
    void doVarification();
    void uamIsLogin();
    void bookingTicket();
private:
    void createUiComponent();
    void createStatusBars();
    void createDockWidget();
    void readSettings();
    void writeSettings();
    void handleReply();
    void formatOutput(const QString &buffer);
    void formatWithColorOutput(const QString &buffer, QColor color);
    void processVarificationResponse(QNetworkReply *reply);
    void processDoVarificationResponse(QNetworkReply *reply);
    void processQueryTicketResponse(QNetworkReply *reply);
    void processStationNameTxtResponse(QNetworkReply *reply);
    void processUserLoginResponse(QNetworkReply *reply);
    void processPassportUamtkResponse(QNetworkReply *reply);
    void processPassportUamtkClientResponse(QNetworkReply *reply);
    void processUserIsLoginResponse(QNetworkReply *reply);
private:
    Ui::MainWindow *ui;
    QTableView *tableView;
    QStandardItemModel *model;
    QDialog *loginDialog;
    QDialog *SettingDialog;
    QTextBrowser *browser;
};

#endif // MAINWINDOW_H

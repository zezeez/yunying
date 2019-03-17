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
#include <QTableWidget>

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
    void showSettingDialog();
    void showPassengerDialog();
    void refreshVarificationImage();
    void submitLoginRequest();
    void queryTicket();
    void swapStation();
    void doVarification();
    void uamIsLogin();
    void bookingTicket();
    void showSelectPassengerDialog();
    void showSelectTrainNoDialog();
    void showSelectSeatTypeDialog();
    void setSelectedPassenger();
    void setUnselectedPassenger();
    void selectPassengerDialogClosed();
    void setSelectedTrainNo();
    void setUnselectedTrainNo();
    void selectTrainNoDialogClosed();
    void setSelectedSeatType();
    void setUnselectedSeatType();
    void selectSeatTypeDialogClosed();
    void rightMenuSelectTrainNo();
    void rightMenuSelectTrainNoAll();
    void rightMenuUnselectTrainNo();
    void rightMenuUnselectTrainNoAll();

private:
    void createUiComponent();
    void createStatusBars();
    void createDockWidget();
    void createRightMenu();
    void readSettings();
    void writeSettings();
    void handleReply();
    int replyIsOk(QNetworkReply *reply);
    void formatOutput(const QString &output);
    void commonSetting(QTabWidget *tab);
    void grabTicketSetting(QTabWidget *tab);
    void formatWithColorOutput(const QString &output, QColor color);
    void processVarificationResponse(QNetworkReply *reply);
    void processDoVarificationResponse(QNetworkReply *reply);
    void processQueryTicketResponse(QNetworkReply *reply);
    void processStationNameTxtResponse(QNetworkReply *reply);
    void processUserLoginResponse(QNetworkReply *reply);
    void processPassportUamtkResponse(QNetworkReply *reply);
    void processPassportUamtkClientResponse(QNetworkReply *reply);
    void processUserIsLoginResponse(QNetworkReply *reply);
    void processGetPassengerInfoResponse(QNetworkReply *reply);
    void processGetPassengerInitDcResponse(QNetworkReply *reply);
    void processCheckUserResponse(QNetworkReply *reply);
    void processSubmitOrderRequestResponse(QNetworkReply *reply);
    void processCheckOrderInfoResponse(QNetworkReply *reply);
private:
    Ui::MainWindow *ui;
    QTableView *tableView;
    QDialog *loginDialog;
    QDialog *settingDialog;
    QDialog *passengerDialog;
    QDialog *trainNoDialog;
    QDialog *seatTypeDialog;
    QTextBrowser *browser;
    QMenu *rightMenu;
};

#endif // MAINWINDOW_H

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
#include <QMap>
#include <QStringList>
#include <QString>
#include <QProgressBar>
#include <QDateEdit>
#include <QMediaPlayer>
#include "passengerdialog.h"
#include "trainnodialog.h"
#include "seattypedialog.h"
#include "settingdialog.h"
#include "logindialog.h"
#include "seatdialog.h"
#include "chart/barchart.h"
#include "chart/linechart.h"

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
    void queryTicket();
    void swapStation();
    void addTrainToSelected();
    void rightMenuSelectTrain();
    void rightMenuSelectTrainAll();
    void rightMenuUnselectTrain();
    void rightMenuUnselectTrainAll();

public:
    void createUiComponent();
    void initStatusBars();
    void createDockWidget();
    void createHistoryInfoDialog();
    void createRightMenu();
    void readSettings();
    void writeSettings();
    void uamLogined();
    void uamNotLogined();
    int replyIsOk(QNetworkReply *reply);
    void showStatusBarMessage(const QString &message);
    void formatOutput(const QString &output);
    void commonSetting(QTabWidget *tab);
    void grabTicketSetting(QTabWidget *tab);
    void formatWithColorOutput(const QString &output, QColor color);
    bool canAddNewTrain(const QString &trainTime);
    void processQueryTicketReply(QVariantMap &data);
    void queryTrainStopStation();
    void processStopStationReply(QVariantMap &data);
    void setStationNameCompleter(const QByteArray &nameText);
    void updateNetQualityStatus(int ms);
    void enterGrabTicketMode();
    void exitGrabTicketMode();
    void enterGrabMode();
    void exitGrabMode();
    void prepareGrabTicket(bool status);
    bool promptBeforeStartGrab();
    void startOrStopGrabTicket();
    void doGrabTicket();
    void switchTableTicketShowType(bool showType);
    void switchTicketShowType();
    void loadStationName();
    void resetLoginDialog();
    void showLoginDialog();
    void showMainWindow();
    void playMusic();
    void stopPlayMusic();
    void startOrStopPlayMusic();
    void setMusicPath(const QString &path);
    void checkUpdateReply(const QVariantMap &varMap);

private:
    Ui::MainWindow *ui;
    QTableView *tableView;
    QTextBrowser *browser;
    QTextBrowser *historyInfoBrower;
    QMenu *rightMenu;

public:
    LoginDialog *loginDialog;
    SettingDialog *settingDialog;
    PassengerDialog *passengerDialog;
    TrainNoDialog *trainNoDialog;
    SeatTypeDialog *seatTypeDialog;
    SeatDialog *seatDialog;
    QDialog *historyInfoDialog;
    QLabel *selectedPassengerTipsLabel;
    QLabel *selectedTrainTipsLabel;
    QLabel *selectedSeatTypeTipsLabel;
    QLabel *selectedSeatTipsLabel;

    QLabel *netQualityIndicateLabel;
    QLabel *netQualityIndicateDescLabel;
    QProgressBar *nextRequestProgress;

    CompleteEdit *fromStationLe;
    CompleteEdit *toStationLe;
    QPushButton *swapStationPb;
    QPushButton *queryTicketPb;
    QPushButton *switchTicketShowTypePb;
    QDateEdit *tourDateDe;
    QPushButton *grabTicketPb;
    int grabTicketInterval;
    QTimer *doGrabTicketTimer;
    QTimer *fixedTimeGrabTimer;
    QTimer *updateProgressBarTimer;
    QVector<QString> queryStopStionArgs;

    QMediaPlayer *player;
    QPushButton *playMusicPb;
    QTimer *stopMusicTimer;

    QTimer *skipMaintenanceTimer;

    BarChartView *statChart;
    LineChartView *delayChart;
};

#endif // MAINWINDOW_H

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
#include <QProgressDialog>
#include "passengerdialog.h"
#include "trainnodialog.h"
#include "seattypedialog.h"
#include "settingdialog.h"
#include "logindialog.h"
#include "seatdialog.h"
#include "chart/barchart.h"
#include "chart/linechart.h"
#include "lib/ntp/include/ntp.h"
#include "selltimequerydialog.h"
#include "upgrademanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setRemainTicketColor(QString &remain, QStandardItem *item, bool canCandidate);

public slots:
    void userStartStationChanged();
    void userEndStationChanged();
    void userTourDateChanged(const QDate &date);
protected:
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void about();
    void onlineHelp();
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
    void syncTime();
    void uamLogined();
    void uamNotLogined();
    void logout();
    void logoutSuccess();
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
    void updateLoginButtonStatus(bool isLogin);
    void updateLatencyChart(int d);
#ifdef HAS_CDN
    void updateAvaliableCdnNum(int num);
#endif
    // update
    void checkUpdateReply(const QVariantMap &varMap);
    void confirmUpdate();

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
    SellTimeQueryDialog *selltimeDialog;
    QLabel *selectedPassengerTipsLabel;
    QLabel *selectedTrainTipsLabel;
    QLabel *selectedSeatTypeTipsLabel;
    QLabel *selectedSeatTipsLabel;
    QPushButton *loginButton;
#ifdef HAS_CDN
    QLabel *cdnIndicatorLabel;
#endif
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
    LineChartView *latencyChart;

    Ntp ntp;

    UpgradeManager upgradeMng;
};

#endif // MAINWINDOW_H

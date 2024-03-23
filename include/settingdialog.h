#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QCheckBox>
#include "lib/smtp/include/smtp.h"
#include "userdata.h"

class SettingDialog : public QDialog
{
    Q_OBJECT
public:
    SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();
    void commonSetting(QTabWidget *tab);
    void grabTicketSetting(QTabWidget *tab);
    void setUp();
    void setSelectedTrainNo();
    void setUnselectedTrainNo();
    void candidateSetting(QTabWidget *tab);
    void sendMailWork(const QString &mailMsg);
    void sendMail(const QString &mailMsg);
    void sendTestMail();
    void notifySetting(QTabWidget *tab);
    void wxNotifySetting(QTabWidget *tab);
    bool isShortMode();
    bool isRandomMode();
    bool isFixedTimeMode();
    bool isCustomMode();
    bool setQueryTicketMode(enum GRABTICKETMODEE mode);
    void sendWxNotify(const QString &msg);
    void sendWxNotifyReply(QVariantMap &varMap);
    void queryWxNotifyStatus();
    void queryWxNotifyStatusReply(QVariantMap &varMap);

public:
    QVector<bool> trainTypeShowVec;
    QLabel *emailTestMsgLa;
    QTimer *emailTestTimer;
    SimpleSmtpEmail *smtpEmail;

    QRadioButton *shortRb;
    QRadioButton *randomRb;
    QRadioButton *fixTimeRb;
    QRadioButton *customRb;

    QLabel *wxNotifyLabel;
    QString notifyPushId;
    QString notifyReadKey;
    QTimer *notifyStatusTimer;

//#ifdef HAS_CDN
    QCheckBox *cdnCb;
//#endif
};

#endif // SETTINGDIALOG_H

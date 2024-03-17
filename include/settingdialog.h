#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include "lib/smtp/include/smtp.h"

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
    void sendTestEmail();
    void notifySetting(QTabWidget *tab);

public Q_SLOTS:
    void show();
    void hide();

public:
    QDialog *dialog;
    QVector<bool> trainTypeShowVec;
    QLabel *emailTestMsgLa;
    QTimer *emailTestTimer;
    SimpleSmtpEmail *smtpEmail;
};

#endif // SETTINGDIALOG_H

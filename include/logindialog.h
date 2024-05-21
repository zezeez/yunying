#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <completeedit.h>
#include <QTabWidget>
#include "serverip.h"

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    void setUp();
    void tabIndexChanged(int index);
    void qrCodeLabelClicked();
    void selectQrCodeTab();
    void hideQrCodeTab();
    void showQrCodeTab();
    void selectPasswordTab();
    void smsRefreshLimit();
    void showSmsVerification();
    void hideSmsVerification();
    void hideUserNamePasswd();
    void showUserNamePasswd();
    void onLogin();
    void onSmsVerificationLogin();
    bool verifyInput();
    void reset();
    void qrCodeRefreshTimeout();
    void showLoadingQrCode();
    void showLoadedQrCode(const QVariantMap &varMap);
    void updateQrCodeStatus(int status);
#ifdef HAS_CDN
    void addLoginServer(const QString &ip, const QString &location);
    void setLoginServerLocation(const QString &ip, const QString &location);
    void setSeletedLoginServer(int index);
#endif

public Q_SLOTS:
    void idCardNumLeTextChanged(const QString &text);
public:
    QTabWidget *tabWidget;
    QWidget *qrCodeWidget;
    QLabel *logoLabel;
#ifdef HAS_CDN
    QLabel *loginServerLa;
    QComboBox *loginServerCb;
#endif
    QLabel *userNameLa;
    QLineEdit *userNameLe;
    QLabel *passwdLa;
    QLineEdit *passwdLe;
    QCheckBox *keepPasswdCB;
    QLineEdit *idCardNumLe;
    QLineEdit *smsLe;
    QPushButton *smsPb;
    ClickLabel *qrCodeLabel;
    QLabel *tipsLabel;
    QLabel *remindLabel1;
    QLabel *remindLabel2;
    QPushButton *loginPb;
    QTimer *qrCodeRefreshTimer;
    QString qrCodeUuid;
#ifdef HAS_CDN
    ServerIp sip;
#endif
};

#endif // LOGINDIALOG_H

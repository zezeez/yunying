#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <completeedit.h>
#include <QTabWidget>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    LoginDialog(QDialog *parent = nullptr);
    ~LoginDialog();
    void setUp();
    void tabIndexChanged(int index);
    void qrCodeLabelClicked();
    void selectQrCodeTab();
    void hideQrCodeTab();
    void showQrCodeTab();
    void selectPasswordTab();
    void smsPbClicked();
    void showSmsVerification();
    void hideSmsVerification();
    void hideUserNamePasswd();
    void showUserNamePasswd();
    void onLogin();
    void onSmsVerificationLogin();
    bool verifyInput();
    void reset();
    void showLoadingQrCode();
    void showLoadedQrCode(const QVariantMap &varMap);
    void updateQrCodeStatus(int status);

public Q_SLOTS:
    void idCardNumLeTextChanged(const QString &text);
public:
    QTabWidget *tabWidget;
    QWidget *qrCodeWidget;
    QLabel *logoLabel;
    QLineEdit *userNameLe;
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
};

#endif // LOGINDIALOG_H

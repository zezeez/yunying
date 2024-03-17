#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <completeedit.h>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    LoginDialog(QDialog *parent = nullptr);
    ~LoginDialog();
    void show();
    void hide();
    void setUp();
    void tabIndexChanged(int index);
    void qrCodeLabelClicked();
    void selectQrCodeTab();
    void hideQrCodeTab();
    void showQrCodeTab();
    void selectPasswordTab();
    void smsPbClicked();
    void showSmsVerification();
    void hideUserNamePasswd();
    void onLogin();
    void onSmsVerificationLogin();
    bool verifyInput();
public Q_SLOTS:
    void idCardNumLeTextChanged(const QString &text);
public:
    QDialog *dialog;
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
};

#endif // LOGINDIALOG_H

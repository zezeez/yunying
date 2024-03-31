#include "logindialog.h"
#include "userdata.h"
#include "completeedit.h"
#include "nethelper.h"
#include <QWidget>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>
#include <QPixmap>
#include <QCheckBox>
#include <QRect>
#include <QSettings>
#include <QMovie>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowIcon(QIcon(_(":/icon/images/ticket.ico")));
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::setUp()
{
    tabWidget = new QTabWidget;
    QSettings setting;
    userNameLe = new QLineEdit;
    passwdLe = new QLineEdit;
    passwdLe->setEchoMode(QLineEdit::Password);
    logoLabel = new QLabel;
    QPixmap pixmap;
    pixmap.load(QStringLiteral(":/icon/images/yy.png"));
    logoLabel->setPixmap(pixmap);
    logoLabel->setAlignment(Qt::AlignCenter);
    QFont font;
    remindLabel1 = new QLabel;
    remindLabel1->setStyleSheet(_("color: red"));
    font.setPointSize(10);
    remindLabel1->setFont(font);
    remindLabel1->setMargin(0);
    remindLabel1->setFixedSize(200, 20);

    QFormLayout *fLayout = new QFormLayout;
    fLayout->addRow(logoLabel);
    fLayout->addRow(tr("账号："), userNameLe);
    fLayout->addRow(tr("密码："), passwdLe);
    fLayout->addRow(tr(""), remindLabel1);
    fLayout->setFormAlignment(Qt::AlignHCenter);
    fLayout->setSpacing(15);

    //UserData *ud = UserData::instance();
    //struct UserLoginInfo loginInfo = ud->getUserLoginInfo();
    userNameLe->setPlaceholderText(tr("12306用户名/邮箱/手机号"));
    userNameLe->setFocus();
    //userNameLe->setText(loginInfo.account);
    passwdLe->setPlaceholderText(tr("密码"));
    //passwdLe->setText(loginInfo.passwd);
    keepPasswdCB = new QCheckBox;
    keepPasswdCB->setText(tr("记住密码"));

    QVBoxLayout *vLayout = new QVBoxLayout;
    //fLayout->addWidget(logoLabel);
    vLayout->addLayout(fLayout);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(keepPasswdCB);
    hLayout->setAlignment(Qt::AlignCenter);
    vLayout->addLayout(hLayout);

    //vLayout->addWidget(label);
    //vLayout->setMargin(20);

    hLayout = new QHBoxLayout;
    smsLe = new QLineEdit;
    idCardNumLe = new QLineEdit;
    connect(idCardNumLe, &QLineEdit::textChanged, this, &LoginDialog::idCardNumLeTextChanged);
    idCardNumLe->hide();
    smsPb = new QPushButton;
    smsPb->setText(tr("获取验证码"));
    connect(smsPb, &QPushButton::clicked, this, [=]() {
        smsPb->setDisabled(true);
        QTimer *timer = new QTimer;
        timer->setInterval(1000);
        connect(timer, &QTimer::timeout, this, [=]() {
            static int count = 60;
            if (--count == 0) {
                if (idCardNumLe->text().trimmed().length() == 4)
                    smsPb->setEnabled(true);
                smsPb->setText(tr("获取验证码"));
                timer->stop();
                delete timer;
                count = 60;
            } else {
                smsPb->setText(QString::number(count));
            }
        });
        smsPb->setText(QStringLiteral("60"));
        timer->start();
        smsLe->setFocus();
        NetHelper::instance()->sendSmsRequest(idCardNumLe->text().trimmed());
    });
    smsPb->hide();
    smsLe->hide();
    smsPb->setMinimumHeight(20);
    smsLe->setMinimumHeight(20);
    idCardNumLe->setMinimumHeight(20);
    hLayout->addWidget(smsLe);
    hLayout->addWidget(smsPb);
    //vLayout->addSpacing(20);
    vLayout->addWidget(idCardNumLe);
    //vLayout->addSpacing(10);
    vLayout->addLayout(hLayout);
    remindLabel2 = new QLabel;
    remindLabel2->setStyleSheet(_("color: red"));
    font.setPointSize(10);
    remindLabel2->setFont(font);
    remindLabel2->setFixedSize(200, 20);
    vLayout->addWidget(remindLabel2);
    vLayout->addStretch();

    loginPb = new QPushButton;
    //button->setText(tr("刷新"));
    //connect(button, &QPushButton::clicked, this, &MainWindow::refreshVarificationImage);
    //hLayout->addWidget(button);
    //hLayout->addStretch();

    //button = new QPushButton;
    loginPb->setText(tr("登陆"));
    connect(loginPb, &QPushButton::clicked, this, &LoginDialog::onLogin);
    hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(loginPb);
    hLayout->addStretch();

    bool checked = setting.value(_("login/keep_passwd"), false).value<bool>();
    if (checked) {
        QString text = setting.value(_("login/username"), _("")).value<QString>();
        userNameLe->setText(text);
        text = setting.value(_("login/passwd"), _("")).value<QString>();
        passwdLe->setText(text);
        loginPb->setFocus();
    }
    keepPasswdCB->setChecked(checked);

    vLayout->addLayout(hLayout);
    vLayout->setAlignment(Qt::AlignHCenter);

    //NetHelper *nHelper = NetHelper::instance();
    //nHelper->getVarificationImage();

    QWidget *widget = new QWidget;
    widget->setLayout(vLayout);
    tabWidget->addTab(widget, tr("密码登陆"));

    qrCodeLabel = new ClickLabel;
    tipsLabel = new QLabel;
    qrCodeLabel->setAlignment(Qt::AlignCenter);
    tipsLabel->setAlignment(Qt::AlignCenter);
    vLayout = new QVBoxLayout;
    vLayout->addWidget(qrCodeLabel);
    vLayout->addWidget(tipsLabel);
    vLayout->setAlignment(Qt::AlignCenter);
    qrCodeWidget = new QWidget;
    qrCodeWidget->setLayout(vLayout);
    tabWidget->addTab(qrCodeWidget, tr("扫码登陆"));

    hLayout = new QHBoxLayout;
    hLayout->addWidget(tabWidget);
    setLayout(hLayout);
    resize(QSize(300, 300));
    connect(tabWidget, &QTabWidget::currentChanged, this, &LoginDialog::tabIndexChanged);
    connect(qrCodeLabel, &ClickLabel::clicked, this, &LoginDialog::qrCodeLabelClicked);
    //NetHelper::instance()->initLoginCookie();
    //NetHelper::instance()->getLoginConf();

    UserData::instance()->runStatus = EIDLE;
    qrCodeRefreshTimer = nullptr;
    //show();
}

void LoginDialog::tabIndexChanged(int index)
{
    QPixmap pixmap = qrCodeLabel->pixmap();

    if (index == 1 && pixmap.isNull()) {
        NetHelper::instance()->createQrCode();
    }
}

void LoginDialog::qrCodeLabelClicked()
{
    NetHelper::instance()->createQrCode();
}

void LoginDialog::selectQrCodeTab()
{
    tabWidget->setCurrentIndex(1);
}

void LoginDialog::hideQrCodeTab()
{
    if (tabWidget->count() == 2) {
        tabWidget->setCurrentIndex(0);
        tabWidget->removeTab(1);
    }
}

void LoginDialog::showQrCodeTab()
{
    if (tabWidget->count() == 1) {
        tabWidget->addTab(qrCodeWidget, tr("扫码登陆"));
        tabWidget->setCurrentIndex(1);
    }
}

void LoginDialog::selectPasswordTab()
{
    tabWidget->setCurrentIndex(0);
}

void LoginDialog::idCardNumLeTextChanged(const QString &text)
{
    if (text.length() == 4 && smsPb->text() == QStringLiteral("获取验证码")) {
        smsPb->setEnabled(true);
    } else {
        smsPb->setDisabled(true);
    }
}

void LoginDialog::showSmsVerification()
{
    QWidget *widget = tabWidget->widget(0);
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(widget->layout());

    loginPb->setText(tr("提交"));
    disconnect(loginPb, &QPushButton::clicked, nullptr, nullptr);
    connect(loginPb, &QPushButton::clicked, this, &LoginDialog::onSmsVerificationLogin);
    idCardNumLe->setPlaceholderText(tr("请输入登陆账号绑定的证件号后4位"));
    smsLe->setPlaceholderText(tr("短信验证码"));

    idCardNumLe->show();
    smsLe->show();
    smsPb->setDisabled(true);
    smsPb->show();
    selectPasswordTab();
    vLayout->setAlignment(Qt::AlignHCenter);
}

void LoginDialog::hideSmsVerification()
{
    loginPb->setText(tr("登陆"));
    disconnect(loginPb, &QPushButton::clicked, nullptr, nullptr);
    connect(loginPb, &QPushButton::clicked, this, &LoginDialog::onLogin);
    idCardNumLe->hide();
    smsLe->hide();
    smsPb->hide();
    selectPasswordTab();
}

void LoginDialog::hideUserNamePasswd()
{
    QWidget *widget = tabWidget->widget(0);
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(widget->layout());
    QFormLayout *fLayout = static_cast<QFormLayout *>(vLayout->itemAt(0));

    //logoLabel->hide();
    keepPasswdCB->hide();
    remindLabel1->hide();
    userNameLe->hide();
    passwdLe->hide();
    QLabel *label = static_cast<QLabel *>(fLayout->itemAt(1)->widget());
    label->hide();
    label = static_cast<QLabel *>(fLayout->itemAt(3)->widget());
    label->hide();
}

void LoginDialog::showUserNamePasswd()
{
    QWidget *widget = tabWidget->widget(0);
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(widget->layout());
    QFormLayout *fLayout = static_cast<QFormLayout *>(vLayout->itemAt(0));
    //logoLabel->show();
    keepPasswdCB->show();
    remindLabel1->show();
    userNameLe->show();
    passwdLe->show();
    QLabel *label = static_cast<QLabel *>(fLayout->itemAt(1)->widget());
    label->show();
    label = static_cast<QLabel *>(fLayout->itemAt(3)->widget());
    label->show();
}

void LoginDialog::onLogin()
{
    if (verifyInput() == false)
        return;

    QString userName = userNameLe->text().trimmed();
    QString passwd = passwdLe->text().trimmed();
    QSettings setting;
    bool checked = keepPasswdCB->isChecked();
    if (checked) {
        setting.setValue(_("login/username"), userName);
        setting.setValue(_("login/passwd"), passwd);
    }
    setting.setValue(_("login/keep_passwd"), checked);
    NetHelper::instance()->onLogin();
}

void LoginDialog::onSmsVerificationLogin()
{
    if (idCardNumLe->text().trimmed().length() != 4) {
        remindLabel2->setText(tr("请输入正确的登陆账号绑定的证件号后4位"));
        return;
    }
    QString verification_code = smsLe->text().trimmed();
    if (verification_code.isEmpty()) {
        remindLabel2->setText(tr("请输入验证码"));
        return;
    }
    if (verification_code.length() != 6) {
        remindLabel2->setText(tr("请输入正确的验证码"));
        return;
    }
    remindLabel2->setText(tr(""));
    NetHelper::instance()->onSmsVerificationLogin(verification_code);
}

bool LoginDialog::verifyInput()
{
    QString userName = userNameLe->text().trimmed();
    QString passwd = passwdLe->text().trimmed();
    if (userName.isEmpty()) {
        remindLabel1->setText(tr("请输入用户名！"));
        return false;
    }
    if (passwd.isEmpty()) {
        remindLabel1->setText(tr("请输入密码！"));
        return false;
    }
    if (passwd.length() < 6) {
        remindLabel1->setText(tr("密码长度不能少于6位！"));
        return false;
    }
    remindLabel1->setText(tr(""));
    UserData *ud = UserData::instance();
    ud->setUserLoginName(userName);
    ud->setUserLoginPaswd(passwd);
    return true;
}

void LoginDialog::reset()
{
    if (tabWidget->currentIndex() == 0) {
        hideSmsVerification();
        showUserNamePasswd();
    } else {
        NetHelper::instance()->createQrCode();
    }
}

void LoginDialog::qrCodeRefreshTimeout()
{
    NetHelper::instance()->checkQrCode(qrCodeUuid);
}

void LoginDialog::showLoadingQrCode()
{
    QWidget *widget = tabWidget->widget(1);
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(widget->layout());
    QLabel *label = static_cast<QLabel *>(vLayout->itemAt(0)->widget());
    QLabel *tipsLabel = static_cast<QLabel *>(vLayout->itemAt(1)->widget());
    QMovie *movie = label->movie();
    if (!movie) {
        movie = new QMovie(_(":/icon/images/loading.gif"));
        label->setMovie(movie);
        movie->start();
    } else {
        movie->stop();
        movie->start();
    }
    tipsLabel->setText(QStringLiteral("加载中..."));
}

void LoginDialog::showLoadedQrCode(const QVariantMap &varMap)
{
    QWidget *widget = tabWidget->widget(1);
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(widget->layout());
    ClickLabel *label = static_cast<ClickLabel *>(vLayout->itemAt(0)->widget());
    QLabel *tipsLabel = static_cast<QLabel *>(vLayout->itemAt(1)->widget());
    QMovie *movie = label->movie();
    if (movie) {
        label->setMovie(nullptr);
        movie->stop();
        delete movie;
    }
    QPixmap pixMap;
    QByteArray base64Data = varMap[_("image")].toByteArray();
    QByteArray image = QByteArray::fromBase64(base64Data);
    pixMap.loadFromData(image);
    label->setPixmap(pixMap);
    tipsLabel->setText(tr("打开12306手机APP扫描二维码"));
    qrCodeUuid = varMap[_("uuid")].toString();
    if (!qrCodeRefreshTimer) {
        qrCodeRefreshTimer = new QTimer;
        connect(qrCodeRefreshTimer, &QTimer::timeout, this, &LoginDialog::qrCodeRefreshTimeout);
        qrCodeRefreshTimer->setInterval(1500);
    }

    qrCodeRefreshTimer->start();
}

void LoginDialog::updateQrCodeStatus(int status)
{
    QPixmap pixMap;

    // 0：未识别、
    // 1：已识别，暂未授权（未点击授权或不授权）、
    // 2：登录成功，（已识别且已授权）、
    // 3：已失效、
    // 5系统异常
    switch (status) {
    case 0:
        break;
    case 1:
        pixMap.load(QStringLiteral(":/icon/images/login-success.png"));
        qrCodeLabel->setPixmap(pixMap);
        tipsLabel->setText(tr("已扫码，请在12306 APP上点击确认"));
        break;
    case 2:
        NetHelper::instance()->loginIndex();
        NetHelper::instance()->passportUamtk();
        qrCodeRefreshTimer->stop();
        break;
    case 3:
        pixMap.load(QStringLiteral(":/icon/images/qrcode_invalid.png"));
        qrCodeLabel->setPixmap(pixMap);
        tipsLabel->setText(tr("二维码已失效，点击刷新"));
        qrCodeRefreshTimer->stop();
        break;
    case 5:
        pixMap.load(QStringLiteral(":/icon/images/qrcode_invalid.png"));
        qrCodeLabel->setPixmap(pixMap);
        tipsLabel->setText(tr("系统错误，点击刷新"));
        qrCodeRefreshTimer->stop();
        break;
    default:
        pixMap.load(QStringLiteral(":/icon/images/qrcode_invalid.png"));
        qrCodeLabel->setPixmap(pixMap);
        tipsLabel->setText(tr("二维码已失效，点击刷新"));
        qrCodeRefreshTimer->stop();
        break;
    }
}

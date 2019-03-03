#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "userdata.h"
#include "completeedit.h"
#include "nethelper.h"
#include <QDebug>
#include <QCompleter>
#include <QLabel>
#include <QAbstractAnimation>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QMessageBox>
#include <QDateEdit>
#include <QDate>
#include <QTableView>
#include <QTableWidget>
#include <QPushButton>
#include <QDockWidget>
#include <QHeaderView>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QHBoxLayout *hLayout;
    CompleteEdit *cestartSta, *ceEndSta;
    QDateEdit *dateEdit;
    QLabel *label;
    QPushButton *pb;

    QWidget *widget = new QWidget(this);
    hLayout = new QHBoxLayout;
    hLayout->setMargin(10);
    hLayout->setSpacing(20);

    hLayout->addWidget(label = new QLabel(QString("出发站: ")));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hLayout->addWidget(cestartSta = new CompleteEdit);
    cestartSta->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    cestartSta->setMaximumWidth(120);

    hLayout->addWidget(pb = new QPushButton);
    pb->setIcon(QIcon(QStringLiteral(":/icon/images/swap.png")));
    connect(pb, &QPushButton::clicked, this, &MainWindow::swapStation);

    hLayout->addWidget(label = new QLabel(QString("到达站: ")));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hLayout->addWidget(ceEndSta = new CompleteEdit);
    ceEndSta->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    ceEndSta->setMaximumWidth(120);

    hLayout->addWidget(new QLabel(QString("日期: ")));
    hLayout->addWidget(dateEdit = new QDateEdit);
    dateEdit->setMinimumWidth(105);
    hLayout->addStretch();

    hLayout->addWidget(pb = new QPushButton);
    pb->setText(tr("查询"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::queryTicket);
    hLayout->addWidget(pb = new QPushButton);
    pb->setText(tr("登陆"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::uamIsLogin);
    hLayout->addWidget(pb = new QPushButton);
    pb->setText(tr("设置"));
    hLayout->addWidget(pb = new QPushButton);
    pb->setText(tr("开始"));

    QDate curDate = QDate::currentDate();
    dateEdit->setDateRange(curDate, curDate.addDays(30));
    dateEdit->setCalendarPopup(true);
    connect(dateEdit, &QDateEdit::dateChanged, this, &MainWindow::userTourDateChanged);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout);
    setUpTableView();
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->setSpacing(10);
    hLayout2->addWidget(tableView);
    vLayout->addLayout(hLayout2);

    createDockWidget();

    widget->setLayout(vLayout);
    setCentralWidget(widget);

    connect(cestartSta, &CompleteEdit::editingFinished, this, &MainWindow::userStartStationChanged);
    connect(ceEndSta, &CompleteEdit::editingFinished, this, &MainWindow::userEndStationChanged);

    InputCompleter *ic;
    UserData *uData = UserData::instance();

    userTourDateChanged(dateEdit->date());

    ic = new InputCompleter(*uData->getStaMap(), this);
    ic->setCaseSensitivity((Qt::CaseInsensitive));
    cestartSta->setCompleter(ic);
    ic->popup()->setStyleSheet("background-color: #AEEEEE;\
                                color: #000000;\
                                border: 1px solid #7FFFD4;\
                                border-radius: 5px;\
                                padding: 2px 2px 2px 2px;\
                                min-width: 17px;\
                                font: 14px \"Arial\";");
    ic = new InputCompleter(*uData->getStaMap(), this);
    ic->setCaseSensitivity((Qt::CaseInsensitive));
    ceEndSta->setCompleter(ic);
    ic->popup()->setStyleSheet("background-color: #AEEEEE;\
                                color: #000000;\
                                border: 1px solid #7FFFD4;\
                                border-radius: 5px;\
                                padding: 2px 2px 2px 2px;\
                                min-width: 17px;\
                                font: 14px \"Arial\";");

    createUiComponent();

    readSettings();

    handleReply();
    setTabOrder(cestartSta, ceEndSta);
    statusBar()->showMessage(QStringLiteral("准备就绪"));
}

void MainWindow::createDockWidget()
{
    QWidget *widget = new QWidget;
    QDockWidget *dock = new QDockWidget;
    dock->setFeatures(QDockWidget::DockWidgetMovable);
    dock->setFeatures(QDockWidget::DockWidgetClosable);

    addDockWidget(Qt::BottomDockWidgetArea, dock);
    dock->resize(dock->sizeHint());

    browser = new QTextBrowser;
    browser->setTextColor(QColor(0, 139, 139));
    //browser->setMinimumSize(QSize(200, 80));
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(browser);
    widget->setLayout(layout);
    dock->setWidget(widget);
}

void MainWindow::createUiComponent()
{
    QAction *action;
    QMenu *menu;

    menu = menuBar()->addMenu(tr("&File"));

    action = new QAction(tr("&ChangTrain"), this);
    action->setShortcut(tr("Ctrl+T"));
    action->setStatusTip(tr("Change train"));
    connect(action, &QAction::triggered, this, &MainWindow::changeTrain);
    menu->addAction(action);

    action = new QAction(tr("&ChangeStation"), this);
    action->setShortcut(tr("Ctrl+E"));
    action->setStatusTip(tr("Change station"));
    connect(action, &QAction::triggered, this, &MainWindow::changeStation);
    menu->addAction(action);

    action = new QAction(tr("&CancelTicket"), this);
    action->setShortcut(tr("Ctrl+C"));
    action->setStatusTip(tr("Cancel ticket"));
    connect(action, &QAction::triggered, this, &MainWindow::cancelTicket);
    menu->addAction(action);

    action = new QAction(tr("&Exit"), this);
    action->setShortcut(tr("Ctrl+Q"));
    action->setStatusTip(tr("Exit program"));
    connect(action, &QAction::triggered, this, &MainWindow::close);
    menu->addAction(action);

    menu = menuBar()->addMenu(tr("&Setting"));

    action = new QAction(tr("&Setting"), this);
    action->setShortcut(tr("Ctrl+S"));
    action->setStatusTip(tr("Edit configuration"));
    connect(action, &QAction::triggered, this, &MainWindow::editConfiguration);
    menu->addAction(action);

    menu = menuBar()->addMenu(tr("&Help"));

    action = new QAction(tr("About"), this);
    action->setShortcut(QKeySequence::HelpContents);
    action->setStatusTip(tr("Show the about context"));
    connect(action, &QAction::triggered, this, &MainWindow::about);
    menu->addAction(action);

    action = new QAction(tr("About &Qt"), this);
    action->setShortcut(tr("F2"));
    action->setStatusTip(tr("Show the Qt library's About box"));
    connect(action, &QAction::triggered, qApp, &QApplication::aboutQt);
    menu->addAction(action);

    createStatusBars();
}

void MainWindow::userStartStationChanged()
{
    QHBoxLayout *hLayout =  static_cast<QHBoxLayout *>(centralWidget()->layout()->itemAt(0));
    CompleteEdit *pbStart = static_cast<CompleteEdit *>(hLayout->itemAt(1)->widget());
    UserData *ud = UserData::instance();
    QString staFromStation = pbStart->text();
    ud->getUserConfig().staFromName = staFromStation;
    ud->getUserConfig().staFromCode = ud->getStaCode()->value(staFromStation);
}

void MainWindow::userEndStationChanged()
{
    QHBoxLayout *hLayout =  static_cast<QHBoxLayout *>(centralWidget()->layout()->itemAt(0));
    CompleteEdit *pbEnd = static_cast<CompleteEdit *>(hLayout->itemAt(4)->widget());
    UserData *ud = UserData::instance();
    QString staToStation = pbEnd->text();
    ud->getUserConfig().staToName = staToStation;
    ud->getUserConfig().staToCode = ud->getStaCode()->value(staToStation);
}

void MainWindow::userTourDateChanged(const QDate &date)
{
    QString strDate = date.toString(QStringLiteral("yyyy-MM-dd"));
    UserData::instance()->getUserConfig().tourDate = strDate;
}

void MainWindow::swapStation()
{
    QString tmp;
    UserData *ud = UserData::instance();
    UserConfig &uc = ud->getUserConfig();
    QHBoxLayout *hLayout =  static_cast<QHBoxLayout *>(centralWidget()->layout()->itemAt(0));
    CompleteEdit *pbStart = static_cast<CompleteEdit *>(hLayout->itemAt(1)->widget());
    CompleteEdit *pbEnd = static_cast<CompleteEdit *>(hLayout->itemAt(4)->widget());

    tmp = pbStart->text();
    pbStart->setText(pbEnd->text());
    pbEnd->setText(tmp);

    uc.staFromName.swap(uc.staToName);
    uc.staFromCode.swap(uc.staToCode);
}

void MainWindow::handleReply()
{
    NetHelper *helper = NetHelper::instance();
    connect(helper, &NetHelper::finished, [=] (QNetworkReply *reply) {
        ENETRPLYENUM replyType = helper->replyMap.value(reply);
        switch (replyType) {
        case EGETVARIFICATIONCODE:
            processVarificationResponse(reply);
            break;
        case EDOVARIFICATION:
            processDoVarificationResponse(reply);
            break;
        case ELOGIN:
            processUserLoginResponse(reply);
            break;
        case EQUERYTICKET:
            processQueryTicketResponse(reply);
            break;
        case EGETSTATIONNAMETXT:
            processStationNameTxtResponse(reply);
            break;
        case EPASSPORTUAMTK:
            processPassportUamtkResponse(reply);
            break;
        case EPASSPORTUAMTKCLIENT:
            processPassportUamtkClientResponse(reply);
            break;
        case EQUERYLOGINSTATUS:
            processUserIsLoginResponse(reply);
            break;
        default:
            break;
        };
        reply->deleteLater();
    });
}

void MainWindow::setRemainTicketColor(QString &remain, QStandardItem *item)
{
    if (remain.isEmpty())
        return;
    if (!remain.compare(QStringLiteral("有")))
        item->setForeground(QBrush(QColor(0, 255, 0)));
    else if (!remain.compare(QStringLiteral("无")))
        item->setForeground(QBrush(QColor(190, 190, 190)));
    else
        item->setForeground(QBrush(QColor(238, 154, 73)));
}

void MainWindow::bookingTicket()
{
    qDebug() << "booking" << endl;
    QPushButton *button = static_cast<QPushButton *>(sender());
    int num = button->property("row").toInt();
    QString trainNum = button->property("trainNo").toString();
    qDebug() << "num = " << num << ", trainNum = " << trainNum << endl;
}

void MainWindow::processQueryTicketResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302)
    {
        // The target URL if it was a redirect:
        QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        qDebug() << "status code = " << statusCodeInt << "redirection Url is " << redirectionTargetUrl.toString() << endl;
    }
    if (statusCodeInt != 200){
        qDebug() << "Response err: status code = " << statusCodeInt << endl;
        return;
    }

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            if (!response[QLatin1String("status")].toBool()) {
                qDebug() << "Response status error" << endl;
                return;
            }
            QVariantMap data = response[QLatin1String("data")].toMap();
            if (data.isEmpty()) {
                qDebug() << "Response data error: data is empty" << endl;
                return;
            }
            QString flag = data[QLatin1String("flag")].toString();
            if (flag.isEmpty() || flag.compare("1")) {
                qDebug() << "Response flag error: flag = " << flag << endl;
                return;
            }
            QVariantMap stationMap = data[QLatin1String("map")].toMap();
            if (stationMap.isEmpty()) {
                qDebug() << "Response map error: stationMap is empty" << endl;
                return;
            }
            QVariantList resultList = data[QLatin1String("result")].toList();
            if (resultList.isEmpty()) {
                qDebug() << "Response result error: train list is empty" << endl;
                return;
            }
            int size = resultList.size();
            model->setRowCount(size);
            qDebug() << "list size = " << size << endl;
            for (int i = 0; i < size; i++) {
                QString train = resultList[i].toString();
                QStringList trainInfo = train.split('|');
                QStandardItem *item;
                QPushButton *button;

                if (!trainInfo[ETEXTINFO].compare(QStringLiteral("预订"))) {
                    button = new QPushButton(tr("预订"));
                    //button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                    //button->setFixedSize(50, 35);
                    button->setStyleSheet(QStringLiteral("QPushButton { color: #1C86EE; }"));
                    button->setProperty("row", i);
                    button->setProperty("trainNo", trainInfo[ESTATIONTRAINCODE]);
                    connect(button, &QPushButton::clicked, this, &MainWindow::bookingTicket);
                    tableView->setIndexWidget(model->index(i, ETRAINTABLECOLUMNEND - 1), button);
                } else {
                    model->setItem(i, ETRAINNUM, item = new QStandardItem(trainInfo[ESTATIONTRAINCODE]));
                    item->setTextAlignment(Qt::AlignCenter);
                    item->setFont(QFont("Times", 5, QFont::Black));
                    model->setItem(i, EREMARK, item = new QStandardItem(trainInfo[ETEXTINFO]));
                }

                model->setItem(i, ETRAINNUM, item = new QStandardItem(trainInfo[ESTATIONTRAINCODE]));
                item->setTextAlignment(Qt::AlignCenter);
                item->setForeground(QBrush(QColor(99, 184, 255)));
                model->setItem(i, EFROMTOSTATION, item = new QStandardItem(
                                   stationMap.value(trainInfo[EFROMSTATIONTELECODE]).toString() + "\n" +
                                   stationMap.value(trainInfo[ETOSTATIONTELECODE]).toString()));
                item->setTextAlignment(Qt::AlignCenter);
                item->setFont(QFont("Times", 10, QFont::Black));
                //if (!trainInfo[ECANWEBBUY].compare("Y"))
                    //item->setForeground(QBrush(QColor(0, 255, 0)));
                item->setForeground(QBrush(QColor(99, 184, 255)));
                model->setItem(i, ESTARTARRIVETIME, item = new QStandardItem(trainInfo[ESTARTTIME] + "\n" +
                                                                    trainInfo[EARRIVETIME]));
                item->setTextAlignment(Qt::AlignCenter);
                item->setForeground(QBrush(QColor(205, 104, 137)));
                model->setItem(i, EUSEDTIME, item = new QStandardItem(trainInfo[ESPEEDTIME]));
                item->setTextAlignment(Qt::AlignCenter);
                item->setForeground(QBrush(QColor(122, 139, 139)));
                model->setItem(i, ESPECIALSEAT, item = new QStandardItem(trainInfo[ESWZNUM].isEmpty() ?
                                                                    "--" : trainInfo[ESWZNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, EFIRSTPRISEAT, item = new QStandardItem(trainInfo[EZYNUM].isEmpty() ?
                                                                       "--" : trainInfo[EZYNUM]));
                setRemainTicketColor(trainInfo[EZYNUM], item);
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, ESECONDPRISEAT, item = new QStandardItem(trainInfo[EZENUM].isEmpty() ?
                                                                        "--" : trainInfo[EZENUM]));
                setRemainTicketColor(trainInfo[EZENUM], item);
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, EADVSOFTCROUCH, item = new QStandardItem(trainInfo[EGRNUM].isEmpty() ?
                                                                        "--" : trainInfo[EGRNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, ESOFTCROUCH, item = new QStandardItem(trainInfo[ERWNUM].isEmpty() ?
                                                                     "--" : trainInfo[ERWNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, ESTIRCROUCH, item = new QStandardItem(trainInfo[EYBNUM].isEmpty() ?
                                                                     "--" : trainInfo[EYBNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, EHARDCROUCH, item = new QStandardItem(trainInfo[EYWNUM].isEmpty() ?
                                                                     "--" : trainInfo[EYWNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, ESOFTSEAT, item = new QStandardItem(trainInfo[ERZNUM].isEmpty() ?
                                                                   "--" : trainInfo[ERZNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, EHARDSEAT, item = new QStandardItem(trainInfo[EYZNUM].isEmpty() ?
                                                                   "--" : trainInfo[EYZNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, ENOSEAT, item = new QStandardItem(trainInfo[EWZNUM].isEmpty() ?
                                                                 "--" : trainInfo[EWZNUM]));
                item->setTextAlignment(Qt::AlignCenter);
                model->setItem(i, EOTHER, item = new QStandardItem(trainInfo[EQTNUM].isEmpty() ?
                                                                "--" : trainInfo[EQTNUM]));
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::processStationNameTxtResponse(QNetworkReply *reply)
{
    QByteArray text = reply->readAll();
    UserData *ud = UserData::instance();
    ud->writeStationFile(text);

    QHBoxLayout *hLayout =  static_cast<QHBoxLayout *>(centralWidget()->layout()->itemAt(0));
    CompleteEdit *pbStart = static_cast<CompleteEdit *>(hLayout->itemAt(1)->widget());
    CompleteEdit *pbEnd = static_cast<CompleteEdit *>(hLayout->itemAt(4)->widget());

    InputCompleter *ic = new InputCompleter(*ud->getStaMap(), this);
    ic->setCaseSensitivity(Qt::CaseInsensitive);
    pbStart->setCompleter(ic);
    ic = new InputCompleter(*ud->getStaMap(), this);
    ic->setCaseSensitivity(Qt::CaseInsensitive);
    pbEnd->setCompleter(ic);
}

void MainWindow::processUserIsLoginResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302) {
        // The target URL if it was a redirect:
        QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        qDebug() << "status code = " << statusCodeInt << "redirection Url is " << redirectionTargetUrl.toString() << endl;
    }
    if (statusCodeInt != 200) {
        qDebug() << "Response err: status code = " << statusCodeInt << endl;
        return;
    }

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            if (response[QLatin1String("result_code")].toInt() == 0) {
                qDebug() << "logined" << endl;
                NetHelper::instance()->passportUamtk();
            } else {
                showLoginDialog();
            }
        } else {
            showLoginDialog();
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
        showLoginDialog();
    }
}

void MainWindow::processUserLoginResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302) {
        // The target URL if it was a redirect:
        QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        qDebug() << "status code = " << statusCodeInt << "redirection Url is " << redirectionTargetUrl.toString() << endl;
    }
    if (statusCodeInt != 200) {
        qDebug() << "Response err: status code = " << statusCodeInt << endl;
        return;
    }

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            int result_code = response[QLatin1String("result_code")].toInt();
            if (result_code == 0) {
                NetHelper::instance()->passportUamtk();
            } else if (result_code == 1) {
                refreshVarificationImage();
                QFormLayout *fLayout = static_cast<QFormLayout *>(loginDialog->layout()->itemAt(0)->layout());
                QLineEdit *accountEdit = static_cast<QLineEdit *>(fLayout->itemAt(0, QFormLayout::FieldRole)->widget());
                QLineEdit *passwdEdit = static_cast<QLineEdit *>(fLayout->itemAt(1, QFormLayout::FieldRole)->widget());

                QString msg = response[QLatin1String("result_message")].toString();
                if (!msg.isEmpty()) {
                    QMessageBox::warning(this, tr("提示"), msg, QMessageBox::Ok);
                    if (msg.contains("登录名不存在")) {
                        accountEdit->clear();
                        passwdEdit->clear();
                    } else if (msg.contains("密码输入错误")) {
                        passwdEdit->clear();
                    }
                }
                qDebug() << response[QLatin1String("result_message")].toString() << endl;
            } else if (!response[QLatin1String("result_message")].toString().isEmpty()) {
                qDebug() << response[QLatin1String("result_message")].toString() << endl;
                refreshVarificationImage();
            }
            QString uamtk = response[QLatin1String("uamtk")].toString();
            if (!uamtk.isEmpty())
                UserData::instance()->setUamtk(uamtk);
        }
    }
}

void MainWindow::showLoginDialog()
{
    QLineEdit *accountLineEdit = new QLineEdit;
    QLineEdit *passwdLineEdit = new QLineEdit;
    passwdLineEdit->setEchoMode(QLineEdit::Password);
    QFormLayout *fLayout = new QFormLayout;
    fLayout->addRow(tr("账号："), accountLineEdit);
    fLayout->addRow(tr("密码："), passwdLineEdit);

    VarCodeLabel *label = new VarCodeLabel;
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    label->setFixedSize(280, 200);
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(fLayout);
    vLayout->addWidget(label);
    vLayout->setMargin(20);

    QHBoxLayout *hLayout = new QHBoxLayout;

    QPushButton *button = new QPushButton;
    button->setText(tr("刷新"));
    connect(button, &QPushButton::clicked, this, &MainWindow::refreshVarificationImage);
    hLayout->addWidget(button);
    hLayout->addStretch();

    button = new QPushButton;
    button->setText(tr("提交"));
    connect(button, &QPushButton::clicked, this, &MainWindow::doVarification);
    hLayout->addWidget(button);

    vLayout->addLayout(hLayout);

    NetHelper *nHelper = NetHelper::instance();
    nHelper->getVarificationImage();

    loginDialog = new QDialog(this);
    loginDialog->setWindowTitle(QStringLiteral("登陆"));
    //loginDialog.setParent(this);
    loginDialog->setLayout(vLayout);
    loginDialog->exec();
    delete loginDialog;
    loginDialog = nullptr;
}

void MainWindow::processVarificationResponse(QNetworkReply *reply)
{
    if (loginDialog) {
        QPixmap pixMap;
        pixMap.loadFromData(reply->readAll());
        VarCodeLabel *label = static_cast<VarCodeLabel *>(loginDialog->layout()->itemAt(1)->widget());
        label->setPixmap(pixMap);
    }
}

void MainWindow::processDoVarificationResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302) {
        // The target URL if it was a redirect:
        QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        qDebug() << "status code = " << statusCodeInt << "redirection Url is " << redirectionTargetUrl.toString() << endl;
    }
    if (statusCodeInt != 200) {
        qDebug() << "Response err: status code = " << statusCodeInt << endl;
        return;
    }

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            if (!response[QLatin1String("result_code")].toString().compare("4")) {
                qDebug() << "varification successed" << endl;
                submitLoginRequest();
            } else if (!response[QLatin1String("result_message")].toString().isEmpty()) {
                qDebug() << response[QLatin1String("result_message")].toString() << endl;
                refreshVarificationImage();
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }

}

void MainWindow::processPassportUamtkResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302) {
        // The target URL if it was a redirect:
        QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        qDebug() << "status code = " << statusCodeInt << "redirection Url is " << redirectionTargetUrl.toString() << endl;
    }
    if (statusCodeInt != 200) {
        qDebug() << "Response err: status code = " << statusCodeInt << endl;
        return;
    }

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            if (response[QLatin1String("result_code")].toInt() == 0) {
                QString apptk = response[QLatin1String("newapptk")].toString();
                if (!apptk.isEmpty())
                    UserData::instance()->setApptk(apptk);
                NetHelper::instance()->passportUamtkClient(apptk);
            }  else if (!response[QLatin1String("result_message")].toString().isEmpty()) {
                qDebug() << response[QLatin1String("result_message")].toString() << endl;
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::processPassportUamtkClientResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302) {
        // The target URL if it was a redirect:
        QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        qDebug() << "status code = " << statusCodeInt << "redirection Url is " << redirectionTargetUrl.toString() << endl;
    }
    if (statusCodeInt != 200) {
        qDebug() << "Response err: status code = " << statusCodeInt << endl;
        return;
    }

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            if (response[QLatin1String("result_code")].toInt() == 0) {
                qDebug() << "login successed" << endl;
                QString userName = response[QLatin1String("username")].toString();
                if (!userName.isEmpty())
                    UserData::instance()->getUserDetailInfo().usesrName = userName;
                if (loginDialog)
                    loginDialog->close();
                statusBar()->showMessage(QStringLiteral("当前用户：") + userName);
            }  else if (!response[QLatin1String("result_message")].toString().isEmpty()) {
                qDebug() << response[QLatin1String("result_message")].toString() << endl;
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::refreshVarificationImage()
{
    VarCodeLabel *label = static_cast<VarCodeLabel *>(loginDialog->layout()->itemAt(1)->widget());
    label->clearSelected();
    NetHelper::instance()->getVarificationImage();
}

void MainWindow::doVarification()
{
    VarCodeLabel *varLabel = static_cast<VarCodeLabel *>(loginDialog->layout()->itemAt(1)->widget());
    QFormLayout *fLayout = static_cast<QFormLayout *>(loginDialog->layout()->itemAt(0)->layout());
    QLineEdit *accountEdit = static_cast<QLineEdit *>(fLayout->itemAt(0, QFormLayout::FieldRole)->widget());
    QLineEdit *passwdEdit = static_cast<QLineEdit *>(fLayout->itemAt(1, QFormLayout::FieldRole)->widget());
    QString account = accountEdit->text().trimmed();
    QString passwd = passwdEdit->text().trimmed();
    QVector<QPoint> points;
    QVector<mapArea> &ma = varLabel->getPoints();

    for (int i = 0; i < ma.size(); i++) {
        if (ma[i].selected) {
            points.push_back(ma[i].pos);
        }
    }
    if (account.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("username can not be empty."), QMessageBox::Ok);
        return;
    }
    if (passwd.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("password can not be empty."), QMessageBox::Ok);
        return;
    }
    if (points.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("please select varification code."), QMessageBox::Ok);
        return;
    }
    NetHelper::instance()->doVarification(points);
}

void MainWindow::uamIsLogin()
{
    NetHelper::instance()->userIsLogin();
}

void MainWindow::submitLoginRequest()
{
    VarCodeLabel *varLabel = static_cast<VarCodeLabel *>(loginDialog->layout()->itemAt(1)->widget());
    QFormLayout *fLayout = static_cast<QFormLayout *>(loginDialog->layout()->itemAt(0)->layout());
    QLineEdit *accountEdit = static_cast<QLineEdit *>(fLayout->itemAt(0, QFormLayout::FieldRole)->widget());
    QLineEdit *passwdEdit = static_cast<QLineEdit *>(fLayout->itemAt(1, QFormLayout::FieldRole)->widget());
    QString account = accountEdit->text().trimmed();
    QString passwd = passwdEdit->text().trimmed();
    QVector<QPoint> points;
    QVector<mapArea> &ma = varLabel->getPoints();

    for (int i = 0; i < ma.size(); i++) {
        if (ma[i].selected) {
            points.push_back(ma[i].pos);
        }
    }
    varLabel->clearSelected();

    NetHelper::instance()->doLogin(points, account, passwd);
}

void MainWindow::queryTicket()
{
    UserData *ud = UserData::instance();
    UserConfig uc = ud->getUserConfig();
    QString staFromCode = uc.staFromCode;
    QString staFromName = uc.staFromName;
    QString staToCode = uc.staToCode;
    QString staToName = uc.staToName;
    QString tourDate = uc.tourDate;

    qDebug() << "staFromCode = " << staFromCode << ", staToCode = "
             << staToCode << ", tourDate = " << tourDate << endl;
    if (!staFromCode.isEmpty() && !staToCode.isEmpty()) {
        NetHelper::instance()->queryTicket(staFromCode, staToCode, tourDate);
        formatOutput(QStringLiteral("正在查询从") + staFromName + QStringLiteral(" 到 ") + staToName + QStringLiteral("的余票信息"));
    }
}

void MainWindow::setUpTableView()
{
    tableView = new QTableView;
    model = new QStandardItemModel(this);
    tableView->verticalHeader()->setDefaultSectionSize(40);
    tableView->resize(tableView->sizeHint());
    model->setColumnCount(ETRAINTABLECOLUMNEND);
    model->setHeaderData(ETRAINNUM, Qt::Horizontal, tr("车次"));
    model->setHeaderData(EFROMTOSTATION, Qt::Horizontal, tr("出发站\n到达站"));
    model->setHeaderData(ESTARTARRIVETIME, Qt::Horizontal, tr("出发时间\n到达时间"));
    model->setHeaderData(EUSEDTIME, Qt::Horizontal, tr("用时"));
    model->setHeaderData(ESPECIALSEAT, Qt::Horizontal, tr("商务/特等"));
    model->setHeaderData(EFIRSTPRISEAT, Qt::Horizontal, tr("一等座"));
    model->setHeaderData(ESECONDPRISEAT, Qt::Horizontal, tr("二等座"));
    model->setHeaderData(EADVSOFTCROUCH, Qt::Horizontal, tr("高级软卧"));
    model->setHeaderData(ESOFTCROUCH, Qt::Horizontal, tr("软卧"));
    model->setHeaderData(ESTIRCROUCH, Qt::Horizontal, tr("动卧"));
    model->setHeaderData(EHARDCROUCH, Qt::Horizontal, tr("硬卧"));
    model->setHeaderData(ESOFTSEAT, Qt::Horizontal, tr("软座"));
    model->setHeaderData(EHARDSEAT, Qt::Horizontal, tr("硬座"));
    model->setHeaderData(ENOSEAT, Qt::Horizontal, tr("无座"));
    model->setHeaderData(EOTHER, Qt::Horizontal, tr("其他"));
    model->setHeaderData(EREMARK, Qt::Horizontal, tr("备注"));

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    tableView->setSortingEnabled(true);
    tableView->setModel(model);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //列宽随窗口大小改变而改变，每列平均分配，充满整个表，但是此时列宽不能拖动进行改变
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /*int ret = QMessageBox::warning(this,
                                   tr("提示"),
                                   tr("退出程序吗？"),
                                   QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }*/
    writeSettings();
    event->accept();
}

void MainWindow::readSettings()
{
    UserData *ud = UserData::instance();
    if (ud->readConfigFile()) {
        UserConfig &uc = ud->getUserConfig();

        QHBoxLayout *hLayout =  static_cast<QHBoxLayout *>(centralWidget()->layout()->itemAt(0));
        CompleteEdit *pbStart = static_cast<CompleteEdit *>(hLayout->itemAt(1)->widget());
        CompleteEdit *pbEnd = static_cast<CompleteEdit *>(hLayout->itemAt(4)->widget());
        QDateEdit *dateEdit = static_cast<QDateEdit *>(hLayout->itemAt(6)->widget());

        pbStart->setText(uc.staFromName);
        uc.staFromCode = ud->getStaCode()->value(uc.staFromName);
        pbEnd->setText(uc.staToName);
        uc.staToCode = ud->getStaCode()->value(uc.staToName);
        dateEdit->setDate(QDate::fromString(uc.tourDate, "yyyy-MM-dd"));
    }
}

void MainWindow::writeSettings()
{
    UserData::instance()->writeConfigFile();
}

void MainWindow::changeTrain()
{

}

void MainWindow::changeStation()
{

}

void MainWindow::cancelTicket()
{

}

void MainWindow::editConfiguration()
{

}

void MainWindow::createStatusBars()
{
    /*QLabel *label = new QLabel;
    //label->setAlignment(Qt::AlignVCenter);
    //label->setMinimumSize(label->sizeHint());
    statusBar()->addWidget(label);

    label = new QLabel;
    label->setIndent(3);
    statusBar()->addWidget(label);*/
}

void MainWindow::formatOutput(const QString &buffer)
{
    static QString textBuff;
    textBuff.clear();
    QDateTime date = QDateTime::currentDateTime();
    textBuff += date.toString(Qt::ISODate);
    textBuff += " " + buffer;
    browser->append(textBuff);
}

void MainWindow::formatWithColorOutput(const QString &buffer, const QColor color)
{
    static QString textBuff;
    textBuff.clear();
    QDateTime date = QDateTime::currentDateTime();
    textBuff += date.toString(Qt::ISODate);
    textBuff += " " + buffer;
    browser->setTextColor(color);
    browser->append(textBuff);
    browser->setTextColor(QColor(0, 0, 0));
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About 12306 qt client"),
                       tr("<h2>12306 qt client 0.1</h2>"
                          "<p>Copyleft; 2019 Software Inc."
                          "<p>12306 qt client is a client writen"
                          "by third party and publish under GPLv3."
                          ));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete tableView;
    delete model;
}

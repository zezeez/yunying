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
#include "parser.h"
#include <QTableWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QListWidget>

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
    connect(pb, &QPushButton::clicked, this, &MainWindow::showSettingDialog);
    hLayout->addWidget(pb = new QPushButton);
    pb->setText(tr("开始"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::showPassengerDialog);

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
    QGridLayout *gLayout = new QGridLayout;
    QPushButton *pb = new QPushButton;
    QLabel *lb = new QLabel(tr("未选"));
    pb->setText(tr("选择乘车人"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::showSelectPassengerDialog);
    gLayout->addWidget(pb, 0, 0, 1, 1);
    gLayout->addWidget(lb, 0, 1, 1, 1);
    pb = new QPushButton;
    lb = new QLabel(tr("未选"));
    pb->setText(tr("选择车次"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::showSelectTrainNoDialog);
    gLayout->addWidget(pb, 1, 0, 1, 1);
    gLayout->addWidget(lb, 1, 1, 1, 1);
    pb = new QPushButton;
    lb = new QLabel(tr("未选"));
    pb->setText(tr("选择席别"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::showSelectSeatTypeDialog);
    gLayout->addWidget(pb, 2, 0, 1, 1);
    gLayout->addWidget(lb, 2, 1, 1, 1);

    layout->addLayout(gLayout);
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
        NETREQUESTTYPEENUM replyType = helper->replyMap.value(reply);
        Parser parser;
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
        case EGETPASSENGERINFO:
            processGetPassengerInfoResponse(reply);
            break;
        case EPASSENGERINITDC:
            processGetPassengerInitDcResponse(reply);
            break;
        case ECHECKUSER:
            processCheckUserResponse(reply);
            break;
        case ESUBMITORDERREQUEST:
            processSubmitOrderRequestResponse(reply);
            break;
        default:
            break;
        };
        helper->replyMap.remove(reply);
        reply->deleteLater();
    });
}

int MainWindow::replyIsOk(QNetworkReply *reply)
{
    QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302)
    {
        // The target URL if it was a redirect:
        QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        QString output = QStringLiteral("Response err: status code = ") +
                QString::number(statusCodeInt) +
                QStringLiteral("redirection Url is ") +
                redirectionTargetUrl.toString();
        //qDebug() << "Response err: status code = " << statusCodeInt << "redirection Url is " << redirectionTargetUrl.toString() << endl;
        formatWithColorOutput(output, QColor(255, 0, 0));
    }
    if (statusCodeInt != 200){
        qDebug() << "Response err: status code = " << statusCodeInt << endl;
        return -1;
    }
    return 0;
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
    int idx = button->property("row").toInt();
    QString trainNo = button->property("trainNo").toString();
    qDebug() << "num = " << idx << ", trainNum = " << trainNo << endl;
    UserData *ud = UserData::instance();
    ud->setTableViewIdx(idx);
    //QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(tableView->model());
    //QStandardItem *item = itemModel->item(idx);
    NetHelper::instance()->checkUser();
}

void MainWindow::processQueryTicketResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    if (replyIsOk(reply) < 0)
        return;

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
            int can_booking = 0;
            QStandardItemModel *model = static_cast<QStandardItemModel *>(tableView->model());
            model->setRowCount(size);
            UserData *ud = UserData::instance();
            QVector<struct TrainInfo> &trainVec = ud->getTrainInfo();
            trainVec.clear();

            for (int i = 0; i < size; i++) {
                QString train = resultList[i].toString();
                QStringList trainInfo = train.split('|');
                QStandardItem *item;
                QPushButton *button;
                struct TrainInfo info;
                info.securityStr = trainInfo[ESECRETSTR];
                trainVec.push_back(info);

                if (!trainInfo[ETEXTINFO].compare(QStringLiteral("预订"))) {
                    button = new QPushButton(tr("预订"));
                    //button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                    //button->setFixedSize(50, 35);
                    button->setStyleSheet(QStringLiteral("QPushButton { color: #1C86EE; }"));
                    button->setProperty("row", i);
                    //button->setProperty("secStr", trainInfo[ESECRETSTR]);
                    button->setProperty("trainNo", trainInfo[ESTATIONTRAINCODE]);
                    connect(button, &QPushButton::clicked, this, &MainWindow::bookingTicket);
                    tableView->setIndexWidget(model->index(i, ETRAINTABLECOLUMNEND - 1), button);
                    can_booking++;
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
            //UserData *ud = UserData::instance();
            UserConfig uc = ud->getUserConfig();
            QString staFromName = uc.staFromName;
            QString staToName = uc.staToName;
            QString tourDate = uc.tourDate;
            formatOutput(staFromName + QStringLiteral("->") + staToName + QStringLiteral("(") + tourDate +
                         QStringLiteral(") 共计")+ QString::number(size) + QStringLiteral("趟车次, 可预订") +
                         QString::number(can_booking) + QStringLiteral("趟车次"));
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

    if (replyIsOk(reply) < 0)
        return;

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

    if (replyIsOk(reply) < 0)
        return;

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

void MainWindow::processGetPassengerInfoResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    if (replyIsOk(reply) < 0)
        return;

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            QVariantMap data = response[QLatin1String("data")].toMap();
            if (data.isEmpty()) {
                qDebug() << "Response data error: data is empty" << endl;
                return;
            }
            QVariantList list = data[QLatin1String("normal_passengers")].toList();
            if (list.isEmpty()) {
                qDebug() << "Error: passenger list is empty." << endl;
                return;
            }

            UserData *ud = UserData::instance();
            UserDetailInfo &detailInfo = ud->getUserDetailInfo();
            detailInfo.passenger.clear();

            foreach (auto i, list) {
                QVariantMap map = i.toMap();
                struct PassengerInfo pinfo;
                QString idx = map[QLatin1String("code")].toString();
                pinfo.code = idx;

                idx = map[QLatin1String("passenger_name")].toString();
                pinfo.passName = idx;

                idx = map[QLatin1String("passenger_id_type_code")].toString();
                pinfo.passIdTypeCode = idx;

                idx = map[QLatin1String("passenger_id_type_name")].toString();
                pinfo.passIdTypeName = idx;

                idx = map[QLatin1String("passenger_id_no")].toString();
                pinfo.passIdNo = idx;

                idx = map[QLatin1String("passenger_type_name")].toString();
                pinfo.passTypeName = idx;

                idx = map[QLatin1String("mobile_no")].toString();
                pinfo.mobile = idx;

                idx = map[QLatin1String("phone_no")].toString();
                pinfo.phone = idx;

                idx = map[QLatin1String("index_id")].toString();
                pinfo.indexId = idx;

                detailInfo.passenger.push_back(pinfo);
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::processGetPassengerInitDcResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    if (replyIsOk(reply) < 0)
        return;

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::processCheckUserResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    if (replyIsOk(reply) < 0)
        return;

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            QVariantMap data = response[QLatin1String("data")].toMap();
            qDebug() << "response = " << response << endl;
            bool flag = data[QLatin1String("flag")].toBool();
            if (flag) {
                UserData *ud = UserData::instance();
                QVector<struct TrainInfo> info = ud->getTrainInfo();
                int idx = ud->getTableViewIdx();
                Q_ASSERT(idx < info.size());
                UserConfig &uc = ud->getUserConfig();
                NetHelper::instance()->submitOrderRequest(info[idx].securityStr, uc.tourDate,
                                                          uc.staFromName, uc.staToName);
            } else {
                showLoginDialog();
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::processSubmitOrderRequestResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    if (replyIsOk(reply) < 0)
        return;

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            //QVariantMap data = response[QLatin1String("data")].toMap();
            bool status = response[QLatin1String("status")].toBool();

            if (status) {
                qDebug() << "status = " << status << endl;
            } else {
                qDebug() << "Error: submitStatus = " << status << endl;
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::processCheckOrderInfoResponse(QNetworkReply *reply)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);

    if (replyIsOk(reply) < 0)
        return;

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            QVariantMap data = response[QLatin1String("data")].toMap();
            bool submitStatus = data[QLatin1String("submitStatus")].toBool();
            QString ifShowPassCode = data[QLatin1String("ifShowPassCode")].toString();
            if (submitStatus) {
                if (ifShowPassCode.at(0) == 'Y') {
                }
            } else {
                qDebug() << "Error: submitStatus = " << submitStatus << endl;
            }
        }
    } else {
        qDebug() << "Error: " << error.errorString() << endl;
    }
}

void MainWindow::setSelectedPassenger()
{
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(0));
    QVBoxLayout *vLayout2 = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(2));
    QListWidget *unSelected = static_cast<QListWidget *>(vLayout->itemAt(1)->widget());
    QListWidget *selected = static_cast<QListWidget *>(vLayout2->itemAt(1)->widget());

    UserData *ud = UserData::instance();
    struct UserDetailInfo &info = ud->getUserDetailInfo();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QVector<struct PassengerInfo>::const_iterator it;
    QListWidgetItem *item = unSelected->currentItem();
    if (item) {
        QList<QListWidgetItem *> list = selected->findItems(item->text(), Qt::MatchExactly);
        if (list.isEmpty()) {
            for (it = info.passenger.cbegin(); it != info.passenger.cend(); ++it) {
                if (!it->passName.compare(item->text())) {
                    if (!grabSetting.selectedPassenger.contains(*it)) {
                        grabSetting.selectedPassenger.push_back(*it);
                    }
                    break;
                }
            }
            QListWidgetItem *item2 = new QListWidgetItem(selected);
            item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
            selected->setItemSelected(item2, true);
            delete unSelected->takeItem(unSelected->currentRow());
        }
    }

    QVBoxLayout *vLayout1 = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(1));
    QPushButton *pb = static_cast<QPushButton *>(vLayout1->itemAt(1)->widget());
    QPushButton *pb2 = static_cast<QPushButton *>(vLayout1->itemAt(2)->widget());
    if (selected->count() != 0) {
        if (!pb2->isEnabled())
            pb2->setEnabled(true);
    }
    if (unSelected->count() == 0) {
        pb->setEnabled(false);
    }
}

void MainWindow::setUnselectedPassenger()
{
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(0));
    QVBoxLayout *vLayout2 = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(2));
    QListWidget *unSelected = static_cast<QListWidget *>(vLayout->itemAt(1)->widget());
    QListWidget *selected = static_cast<QListWidget *>(vLayout2->itemAt(1)->widget());

    UserData *ud = UserData::instance();
    struct UserDetailInfo &info = ud->getUserDetailInfo();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QVector<struct PassengerInfo>::const_iterator it;
    QListWidgetItem *item = selected->currentItem();

    if (item) {
        for (it = info.passenger.cbegin(); it != info.passenger.cend(); ++it) {
            if (!it->passName.compare(item->text())) {
                if (grabSetting.selectedPassenger.contains(*it)) {
                    grabSetting.selectedPassenger.removeOne(*it);
                }
                break;
            }
        }
        QListWidgetItem *item2 = new QListWidgetItem(unSelected);
        item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
        unSelected->setItemSelected(item2, true);
        delete selected->takeItem(selected->currentRow());
    }

    QVBoxLayout *vLayout1 = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(1));
    QPushButton *pb = static_cast<QPushButton *>(vLayout1->itemAt(2)->widget());
    QPushButton *pb2 = static_cast<QPushButton *>(vLayout1->itemAt(1)->widget());
    if (unSelected->count() != 0) {
        if (!pb2->isEnabled())
            pb2->setEnabled(true);
    }
    if (selected->count() == 0) {
        pb->setEnabled(false);
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

    UserData *ud = UserData::instance();
    struct UserDetailInfo &info = ud->getUserDetailInfo();
    accountLineEdit->setText(info.account);
    passwdLineEdit->setText(info.passwd);

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

void MainWindow::commonSetting(QTabWidget *tab)
{
    QWidget *widget = new QWidget;
    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    QCheckBox *cbAll = new QCheckBox(tr("全选"));
    QCheckBox *cbGTrain = new QCheckBox(tr("G-高铁"));
    QCheckBox *cbDTrain = new QCheckBox(tr("D-动车"));
    QCheckBox *cbCTrain = new QCheckBox(tr("C-城际"));
    QCheckBox *cbZTrain = new QCheckBox(tr("Z-直达"));
    QCheckBox *cbTTrain = new QCheckBox(tr("T-特快"));
    QCheckBox *cbKTrain = new QCheckBox(tr("K-普快"));
    QCheckBox *cbOTrain = new QCheckBox(tr("其他"));

    connect(cbAll, &QCheckBox::stateChanged, this, [=](int state) {
        cbGTrain->setChecked(state);
        cbDTrain->setChecked(state);
        cbCTrain->setChecked(state);
        cbZTrain->setChecked(state);
        cbTTrain->setChecked(state);
        cbKTrain->setChecked(state);
        cbOTrain->setChecked(state);
    });
    cbAll->setChecked(true);
    hLayout->addWidget(cbAll);
    hLayout->addWidget(cbGTrain);
    hLayout->addWidget(cbDTrain);
    hLayout->addWidget(cbCTrain);
    hLayout->addWidget(cbZTrain);
    hLayout->addWidget(cbTTrain);
    hLayout->addWidget(cbKTrain);
    hLayout->addWidget(cbOTrain);
    hLayout->setSpacing(10);
    hLayout->setMargin(5);
    hLayout->addStretch(1);

    QGroupBox *group = new QGroupBox;
    group->setLayout(hLayout);
    group->setTitle(tr("筛选"));

    vLayout->addWidget(group);
    widget->setLayout(vLayout);
    tab->addTab(widget, tr("一般设置"));
}

void MainWindow::grabTicketSetting(QTabWidget *tab)
{
    QWidget *widget = new QWidget;
    //QHBoxLayout *hLayout = new QHBoxLayout;
    //QVBoxLayout *vLayout = new QVBoxLayout;
    QGridLayout *gLayout = new QGridLayout;
    QLabel *label = new QLabel(tr("定时抢票"));
    QTimeEdit *timeEdit = new QTimeEdit;
    timeEdit->setTime(QTime(6, 0, 0));
    gLayout->addWidget(label, 0, 0, 1, 1);
    gLayout->addWidget(timeEdit, 0, 1, 1, 1);
    gLayout->setRowStretch(0, 1);

    //vLayout->addLayout(hLayout);
    widget->setLayout(gLayout);
    tab->addTab(widget, tr("抢票设置"));
}

void MainWindow::showSettingDialog()
{
    QTabWidget *tabWidget = new QTabWidget;
    settingDialog = new QDialog(this);
    QVBoxLayout *vLayout = new QVBoxLayout;

    commonSetting(tabWidget);
    grabTicketSetting(tabWidget);

    vLayout->addWidget(tabWidget);
    settingDialog->setLayout(vLayout);
    //settingDialog->resize(800, 600);
    settingDialog->exec();
    delete settingDialog;
    settingDialog = nullptr;
}

void MainWindow::showPassengerDialog()
{

}

void MainWindow::showSelectPassengerDialog()
{
    passengerDialog = new QDialog(this);
    QListWidget *unSelected = new QListWidget;
    QListWidget *selected = new QListWidget;
    QLabel *label = new QLabel(tr("未选中乘车人："));

    //unSelected->setViewMode(QListWidget::IconMode );
    //unSelected->setResizeMode(QListWidget::Adjust);
    //unSelected->setMovement(QListWidget::Static);
    UserData *ud = UserData::instance();
    struct UserDetailInfo &info = ud->getUserDetailInfo();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();
    QVector<struct PassengerInfo>::const_iterator it;

    for (it = grabSetting.selectedPassenger.cbegin();
         it != grabSetting.selectedPassenger.cend(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(selected);
        //QStyle::StandardPixmap sp = static_cast<QStyle::StandardPixmap>(i % 57);
        //item->setData(Qt::DecorationRole, qApp->style()->standardPixmap(sp).scaled(QSize(16,16), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        item->setData(Qt::DisplayRole, QObject::tr("%1").arg(it->passName));
    }
    for (it = info.passenger.cbegin();
         it != info.passenger.cend(); ++it) {
        if (!grabSetting.selectedPassenger.contains(*it)) {
            QListWidgetItem *item = new QListWidgetItem(unSelected);
            //QStyle::StandardPixmap sp = static_cast<QStyle::StandardPixmap>(i % 57);
            //item->setData(Qt::DecorationRole, qApp->style()->standardPixmap(sp).scaled(QSize(16,16), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            item->setData(Qt::DisplayRole, QObject::tr("%1").arg(it->passName));
        }
    }

    unSelected->setCurrentRow(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(label);
    vLayout->addWidget(unSelected);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addStretch();
    QPushButton *pb = new QPushButton(tr(">>"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::setSelectedPassenger);
    pb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(pb);

    pb = new QPushButton(tr("<<"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::setUnselectedPassenger);
    pb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(pb);
    vLayout2->addStretch();

    pb = new QPushButton(tr("确定"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::selectPassengerDialogClosed);
    vLayout2->addWidget(pb);

    QVBoxLayout *vLayout3 = new QVBoxLayout;
    label = new QLabel(tr("已选中乘车人："));
    vLayout3->addWidget(label);
    vLayout3->addWidget(selected);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    passengerDialog->setLayout(hLayout);
    passengerDialog->setWindowTitle(tr("选择乘车人"));
    //passengerDialog->resize(350, 200);
    passengerDialog->exec();

    delete passengerDialog;
    passengerDialog = nullptr;
}

void MainWindow::selectPassengerDialogClosed()
{
    /*qDebug() << "closed" << endl;
    //QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(0));
    QVBoxLayout *vLayout2 = static_cast<QVBoxLayout *>(passengerDialog->layout()->itemAt(2));
    //QListWidget *unSelected = static_cast<QListWidget *>(vLayout->itemAt(1)->widget());
    QListWidget *selected = static_cast<QListWidget *>(vLayout2->itemAt(1)->widget());

    UserData *ud = UserData::instance();
    struct UserDetailInfo &info = ud->getUserDetailInfo();
    info.selectedPassenger.clear();

    for (int i = 0; i < selected->count(); ++i) {
        QListWidgetItem *item = selected->item(i);
        QVector<struct PassengerInfo>::iterator it;
        for (it = info.passenger.begin(); it != info.passenger.end(); ++it) {
            if (!it->passName.compare(item->text())) {
                if (!info.selectedPassenger.contains(*it)) {
                    info.selectedPassenger.push_back(*it);
                }
                break;
            }
        }
    }*/
    passengerDialog->close();
}

void MainWindow::setSelectedTrainNo()
{
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(trainNoDialog->layout()->itemAt(0));
    QVBoxLayout *vLayout2 = static_cast<QVBoxLayout *>(trainNoDialog->layout()->itemAt(2));
    QListWidget *unSelected = static_cast<QListWidget *>(vLayout->itemAt(1)->widget());
    QListWidget *selected = static_cast<QListWidget *>(vLayout2->itemAt(1)->widget());

    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QListWidgetItem *item = unSelected->currentItem();
    if (item) {
        QList<QListWidgetItem *> list = selected->findItems(item->text(), Qt::MatchExactly);
        if (list.isEmpty()) {
            if (!grabSetting.trainNo.contains(item->text())) {
                grabSetting.trainNo.push_back(item->text());
            }
            QListWidgetItem *item2 = new QListWidgetItem(selected);
            item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
            selected->setItemSelected(item2, true);
            delete unSelected->takeItem(unSelected->currentRow());
        }
    }

    QVBoxLayout *vLayout1 = static_cast<QVBoxLayout *>(trainNoDialog->layout()->itemAt(1));
    QPushButton *pb = static_cast<QPushButton *>(vLayout1->itemAt(1)->widget());
    QPushButton *pb2 = static_cast<QPushButton *>(vLayout1->itemAt(2)->widget());
    if (selected->count() != 0) {
        if (!pb2->isEnabled())
            pb2->setEnabled(true);
    }
    if (unSelected->count() == 0) {
        pb->setEnabled(false);
    }
}

void MainWindow::setUnselectedTrainNo()
{
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(trainNoDialog->layout()->itemAt(0));
    QVBoxLayout *vLayout2 = static_cast<QVBoxLayout *>(trainNoDialog->layout()->itemAt(2));
    QListWidget *unSelected = static_cast<QListWidget *>(vLayout->itemAt(1)->widget());
    QListWidget *selected = static_cast<QListWidget *>(vLayout2->itemAt(1)->widget());

    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QListWidgetItem *item = selected->currentItem();

    if (item) {
        if (grabSetting.trainNo.contains(item->text())) {
            grabSetting.trainNo.removeOne(item->text());
        }
        QListWidgetItem *item2 = new QListWidgetItem(unSelected);
        item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
        unSelected->setItemSelected(item2, true);
        delete selected->takeItem(selected->currentRow());
    }

    QVBoxLayout *vLayout1 = static_cast<QVBoxLayout *>(trainNoDialog->layout()->itemAt(1));
    QPushButton *pb = static_cast<QPushButton *>(vLayout1->itemAt(2)->widget());
    QPushButton *pb2 = static_cast<QPushButton *>(vLayout1->itemAt(1)->widget());
    if (unSelected->count() != 0) {
        if (!pb2->isEnabled())
            pb2->setEnabled(true);
    }
    if (selected->count() == 0) {
        pb->setEnabled(false);
    }
}

void MainWindow::selectTrainNoDialogClosed()
{
    trainNoDialog->close();
}

void MainWindow::showSelectTrainNoDialog()
{
    trainNoDialog = new QDialog(this);
    QListWidget *unSelected = new QListWidget;
    QListWidget *selected = new QListWidget;
    QLabel *label = new QLabel(tr("未选中车次："));

    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();
    QVector<QString>::const_iterator it;

    for (it = grabSetting.trainNo.cbegin();
         it != grabSetting.trainNo.cend(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(selected);
        item->setData(Qt::DisplayRole, QObject::tr("%1").arg(*it));
    }

    QStandardItemModel *model = static_cast<QStandardItemModel *>(tableView->model());
    for (int i = 0; i < model->rowCount(); ++i) {
    //for (int i = 0; i < 10; i++) {
        QStandardItem *item = model->item(i);
        if (!grabSetting.trainNo.contains(item->text())) {
        //if (!grabSetting.trainNo.contains(tr("%1").arg(i))) {
            QListWidgetItem *witem = new QListWidgetItem(unSelected);
            witem->setData(Qt::DisplayRole, item->text());
            //witem->setData(Qt::DisplayRole, tr("%1").arg(i));
        }
    }

    unSelected->setCurrentRow(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(label);
    vLayout->addWidget(unSelected);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addStretch();
    QPushButton *pb = new QPushButton(tr(">>"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::setSelectedTrainNo);
    pb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(pb);

    pb = new QPushButton(tr("<<"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::setUnselectedTrainNo);
    pb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(pb);
    vLayout2->addStretch();

    pb = new QPushButton(tr("确定"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::selectTrainNoDialogClosed);
    vLayout2->addWidget(pb);

    QVBoxLayout *vLayout3 = new QVBoxLayout;
    label = new QLabel(tr("已选中车次："));
    vLayout3->addWidget(label);
    vLayout3->addWidget(selected);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    trainNoDialog->setLayout(hLayout);
    trainNoDialog->setWindowTitle(tr("选择车次"));
    //trainNoDialog->resize(400, 400);
    trainNoDialog->exec();

    delete trainNoDialog;
    trainNoDialog = nullptr;
}

void MainWindow::setSelectedSeatType()
{
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(seatTypeDialog->layout()->itemAt(0));
    QVBoxLayout *vLayout2 = static_cast<QVBoxLayout *>(seatTypeDialog->layout()->itemAt(2));
    QListWidget *unSelected = static_cast<QListWidget *>(vLayout->itemAt(1)->widget());
    QListWidget *selected = static_cast<QListWidget *>(vLayout2->itemAt(1)->widget());

    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QListWidgetItem *item = unSelected->currentItem();
    if (item) {
        QList<QListWidgetItem *> list = selected->findItems(item->text(), Qt::MatchExactly);
        if (list.isEmpty()) {
            enum ESEATTYPEENUM se = ud->SeatDescToType(item->text());
            if (se != ESEATTYPEINVALID &&
                    !grabSetting.seatType.contains(se)) {
                grabSetting.seatType.push_back(se);
            }
            QListWidgetItem *item2 = new QListWidgetItem(selected);
            item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
            selected->setItemSelected(item2, true);
            delete unSelected->takeItem(unSelected->currentRow());
        }
    }

    QVBoxLayout *vLayout1 = static_cast<QVBoxLayout *>(seatTypeDialog->layout()->itemAt(1));
    QPushButton *pb = static_cast<QPushButton *>(vLayout1->itemAt(1)->widget());
    QPushButton *pb2 = static_cast<QPushButton *>(vLayout1->itemAt(2)->widget());
    if (selected->count() != 0) {
        if (!pb2->isEnabled())
            pb2->setEnabled(true);
    }
    if (unSelected->count() == 0) {
        pb->setEnabled(false);
    }
}

void MainWindow::setUnselectedSeatType()
{
    QVBoxLayout *vLayout = static_cast<QVBoxLayout *>(seatTypeDialog->layout()->itemAt(0));
    QVBoxLayout *vLayout2 = static_cast<QVBoxLayout *>(seatTypeDialog->layout()->itemAt(2));
    QListWidget *unSelected = static_cast<QListWidget *>(vLayout->itemAt(1)->widget());
    QListWidget *selected = static_cast<QListWidget *>(vLayout2->itemAt(1)->widget());

    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QListWidgetItem *item = selected->currentItem();

    if (item) {
        enum ESEATTYPEENUM se = ud->SeatDescToType(item->text());
        if (se != ESEATTYPEINVALID &&
                grabSetting.seatType.contains(se)) {
            grabSetting.seatType.removeOne(se);
        }

        QListWidgetItem *item2 = new QListWidgetItem(unSelected);
        item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
        unSelected->setItemSelected(item2, true);
        delete selected->takeItem(selected->currentRow());
    }

    QVBoxLayout *vLayout1 = static_cast<QVBoxLayout *>(seatTypeDialog->layout()->itemAt(1));
    QPushButton *pb = static_cast<QPushButton *>(vLayout1->itemAt(2)->widget());
    QPushButton *pb2 = static_cast<QPushButton *>(vLayout1->itemAt(1)->widget());
    if (unSelected->count() != 0) {
        if (!pb2->isEnabled())
            pb2->setEnabled(true);
    }
    if (selected->count() == 0) {
        pb->setEnabled(false);
    }
}

void MainWindow::selectSeatTypeDialogClosed()
{
    seatTypeDialog->close();
}

void MainWindow::showSelectSeatTypeDialog()
{
    seatTypeDialog = new QDialog(this);
    QListWidget *unSelected = new QListWidget;
    QListWidget *selected = new QListWidget;
    QLabel *label = new QLabel(tr("未选中席别："));

    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();
    QVector<enum ESEATTYPEENUM>::const_iterator it;

    QVector<enum ESEATTYPEENUM> seat;
    seat.push_back(ESEATSPECIALSEAT);
    seat.push_back(ESEATFIRSTPRISEAT);
    seat.push_back(ESEATSECONDPRISEAT);
    seat.push_back(ESEATADVSOFTCROUCH);
    seat.push_back(ESEATSOFTCROUCH);
    seat.push_back(ESEATSTIRCROUCH);
    seat.push_back(ESEATHARDCROUCH);
    seat.push_back(ESEATSOFTSEAT);
    seat.push_back(ESEATHARDSEAT);
    seat.push_back(ESEATNOSEAT);

    /*QVector<QString> seatDesc;
    seatDesc.push_back(QStringLiteral("特等座"));
    seatDesc.push_back(QStringLiteral("一等座"));
    seatDesc.push_back(QStringLiteral("二等座"));
    seatDesc.push_back(QStringLiteral("高级软卧"));
    seatDesc.push_back(QStringLiteral("软卧"));
    seatDesc.push_back(QStringLiteral("动卧"));
    seatDesc.push_back(QStringLiteral("硬卧"));
    seatDesc.push_back(QStringLiteral("软座"));
    seatDesc.push_back(QStringLiteral("硬座"));
    seatDesc.push_back(QStringLiteral("无座"));*/

    for (it = grabSetting.seatType.cbegin();
         it != grabSetting.seatType.cend(); ++it) {
        QString sd = ud->seatTypeToDesc(*it);
        if (!sd.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem(selected);
            item->setData(Qt::DisplayRole, sd);
        }
    }

    for (int i = 0; i < seat.size(); i++) {
        if (!grabSetting.seatType.contains(seat[i])) {
            QString sd = ud->seatTypeToDesc(seat[i]);
            if (!sd.isEmpty()) {
                QListWidgetItem *item = new QListWidgetItem(unSelected);
                item->setData(Qt::DisplayRole, sd);
            }
        }
    }

    unSelected->setCurrentRow(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(label);
    vLayout->addWidget(unSelected);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addStretch();
    QPushButton *pb = new QPushButton(tr(">>"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::setSelectedSeatType);
    pb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(pb);

    pb = new QPushButton(tr("<<"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::setUnselectedSeatType);
    pb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(pb);
    vLayout2->addStretch();

    pb = new QPushButton(tr("确定"));
    connect(pb, &QPushButton::clicked, this, &MainWindow::selectSeatTypeDialogClosed);
    vLayout2->addWidget(pb);

    QVBoxLayout *vLayout3 = new QVBoxLayout;
    label = new QLabel(tr("已选中席别："));
    vLayout3->addWidget(label);
    vLayout3->addWidget(selected);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    seatTypeDialog->setLayout(hLayout);
    seatTypeDialog->setWindowTitle(tr("选择席别"));
    //seatTypeDialog->resize(400, 400);
    seatTypeDialog->exec();

    delete seatTypeDialog;
    seatTypeDialog = nullptr;
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

    if (replyIsOk(reply) < 0)
        return;

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

    if (replyIsOk(reply) < 0)
        return;

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

    if (replyIsOk(reply) < 0)
        return;

    if (error.error == QJsonParseError::NoError) {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()) && jsonDocument.isObject()) {
            QVariantMap response = jsonDocument.toVariant().toMap();
            qDebug() << "response = " << response << endl;
            if (response[QLatin1String("result_code")].toInt() == 0) {
                qDebug() << "login successed" << endl;

                QString userName = response[QLatin1String("username")].toString();
                if (!userName.isEmpty())
                    UserData::instance()->getUserDetailInfo().userName = userName;
                if (loginDialog)
                    loginDialog->close();
                statusBar()->showMessage(QStringLiteral("当前用户：") + userName);
                NetHelper::instance()->getPassengerInfo();
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
    UserData *ud = UserData::instance();
    struct UserDetailInfo &info = ud->getUserDetailInfo();

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
    if (info.account.compare(account))
        info.account = account;
    if (info.passwd.compare(passwd))
        info.passwd = passwd;
    NetHelper::instance()->doVarification(points);
}

void MainWindow::uamIsLogin()
{
    NetHelper::instance()->userIsLogin();
}

void MainWindow::submitLoginRequest()
{
    VarCodeLabel *varLabel = static_cast<VarCodeLabel *>(loginDialog->layout()->itemAt(1)->widget());
    //QFormLayout *fLayout = static_cast<QFormLayout *>(loginDialog->layout()->itemAt(0)->layout());
    //QLineEdit *accountEdit = static_cast<QLineEdit *>(fLayout->itemAt(0, QFormLayout::FieldRole)->widget());
    //QLineEdit *passwdEdit = static_cast<QLineEdit *>(fLayout->itemAt(1, QFormLayout::FieldRole)->widget());
    //QString account = accountEdit->text().trimmed();
    //QString passwd = passwdEdit->text().trimmed();
    QVector<QPoint> points;
    QVector<mapArea> &ma = varLabel->getPoints();
    UserData *ud = UserData::instance();
    struct UserDetailInfo &info = ud->getUserDetailInfo();

    for (int i = 0; i < ma.size(); i++) {
        if (ma[i].selected) {
            points.push_back(ma[i].pos);
        }
    }
    varLabel->clearSelected();

    NetHelper::instance()->doLogin(points, info.account, info.passwd);
}

void MainWindow::queryTicket()
{
    UserData *ud = UserData::instance();
    UserConfig uc = ud->getUserConfig();
    QString staFromCode = uc.staFromCode;
    QString staToCode = uc.staToCode;
    QString tourDate = uc.tourDate;

    qDebug() << "staFromCode = " << staFromCode << ", staToCode = "
             << staToCode << ", tourDate = " << tourDate << endl;
    if (!staFromCode.isEmpty() && !staToCode.isEmpty()) {
        NetHelper::instance()->queryTicket(staFromCode, staToCode, tourDate);
    }
}

void MainWindow::rightMenuSelectTrainNo()
{
    QItemSelectionModel *modelSelection = tableView->selectionModel();
    QModelIndexList indexList= modelSelection->selectedIndexes();
    UserData *ud = UserData::instance();
    struct GrabTicketSetting &gs = ud->getGrabTicketSetting();

    foreach (QModelIndex index, indexList) {
        const QStandardItemModel *itemModel = dynamic_cast<const QStandardItemModel *>(index.model());
        const QStandardItem *item = dynamic_cast<const QStandardItem *>(itemModel->item(index.row()));
        if (!gs.trainNo.contains(item->text()))
            gs.trainNo.push_back(item->text());
        qDebug() << item->text() << endl;
    }
}

void MainWindow::rightMenuUnselectTrainNo()
{
    QItemSelectionModel *modelSelection = tableView->selectionModel();
    QModelIndexList indexList= modelSelection->selectedIndexes();
    UserData *ud = UserData::instance();
    struct GrabTicketSetting &gs = ud->getGrabTicketSetting();

    foreach (QModelIndex index, indexList) {
        const QStandardItemModel *itemModel = dynamic_cast<const QStandardItemModel *>(index.model());
        const QStandardItem *item = dynamic_cast<const QStandardItem *>(itemModel->item(index.row()));
        if (gs.trainNo.contains(item->text()))
            gs.trainNo.removeOne(item->text());
    }
}

void MainWindow::rightMenuSelectTrainNoAll()
{
    UserData *ud = UserData::instance();
    struct GrabTicketSetting &gs = ud->getGrabTicketSetting();
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(tableView->model());

    for (int i = 0; i < model->rowCount(); i++) {
        QStandardItem *item = model->item(i);
        if (!gs.trainNo.contains(item->text()))
            gs.trainNo.push_back(item->text());
    }
}

void MainWindow::rightMenuUnselectTrainNoAll()
{
    UserData *ud = UserData::instance();
    struct GrabTicketSetting &gs = ud->getGrabTicketSetting();

    gs.trainNo.clear();
}

void MainWindow::createRightMenu()
{
    rightMenu = new QMenu;
    QAction *select = new QAction("选中车次",this);
    QAction *unSelect = new QAction("取消选中车次",this);
    QAction *selectAll = new QAction("选中所有车次",this);
    QAction *unSelectAll = new QAction("取消选中所有车次",this);

    connect(select, &QAction::triggered, this, &MainWindow::rightMenuSelectTrainNo);
    connect(unSelect, &QAction::triggered, this, &MainWindow::rightMenuUnselectTrainNo);
    connect(selectAll, &QAction::triggered, this, &MainWindow::rightMenuSelectTrainNoAll);
    connect(unSelectAll, &QAction::triggered, this, &MainWindow::rightMenuUnselectTrainNoAll);

    rightMenu->addAction(select);
    rightMenu->addAction(unSelect);
    rightMenu->addAction(selectAll);
    rightMenu->addAction(unSelectAll);
}

void MainWindow::setUpTableView()
{
    tableView = new QTableView;
    QStandardItemModel *model = new QStandardItemModel;
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
    tableView->verticalHeader()->hide();
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);  //少这句，右键没有任何反应的。
    createRightMenu();  //创建一个右键菜单
    connect(tableView, &QTableView::customContextMenuRequested, this, [&] {
        if (tableView->model()->rowCount()) {
            rightMenu->exec(QCursor::pos());
            //qDebug() << "index = " << tableView->currentIndex() << endl;
        }
    });
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

void MainWindow::formatOutput(const QString &output)
{
    static QString textBuff;
    textBuff.clear();
    QDateTime date = QDateTime::currentDateTime();
    textBuff += date.toString(Qt::ISODate);
    textBuff += QStringLiteral(" ") + output;
    browser->append(textBuff);
}

void MainWindow::formatWithColorOutput(const QString &output, const QColor color)
{
    static QString textBuff;
    textBuff.clear();
    QDateTime date = QDateTime::currentDateTime();
    textBuff += date.toString(Qt::ISODate);
    textBuff += QStringLiteral(" ") + output;
    browser->setTextColor(color);
    browser->append(textBuff);
    browser->setTextColor(QColor(0, 0, 0));
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About 12306 qt client"),
                       tr("<h2>12306 qt client 0.1</h2>"
                          "<p>Copyleft; 2019 Software Inc."
                          "<p>12306 qt client is a client writen "
                          "by third party and public under GPLv3."
                          ));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete tableView;
}

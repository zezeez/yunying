#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "userdata.h"
#include "completeedit.h"
#include "nethelper.h"
#include "logindialog.h"
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
#include "analysis.h"
#include <QTableWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QListWidget>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QSettings>
#include "icondelegate.h"
#include "12306.h"

using namespace Qt;

#define _ QStringLiteral

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QHBoxLayout *hLayout;
    QLabel *label;
    QPushButton *pb;

    QWidget *widget = new QWidget(this);
    hLayout = new QHBoxLayout;
    //hLayout->setMargin(10);
    hLayout->setSpacing(20);

    hLayout->addWidget(label = new QLabel(tr("出发站: ")));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hLayout->addWidget(fromStationLe = new CompleteEdit);
    fromStationLe->setPlaceholderText(_("简拼/全拼/汉字"));
    fromStationLe->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    fromStationLe->setMaximumWidth(120);
    connect(fromStationLe, &CompleteEdit::editingFinished, this, &MainWindow::userStartStationChanged);

    hLayout->addWidget(swapStationPb = new QPushButton);
    swapStationPb->setIcon(QIcon(QStringLiteral(":/icon/images/swap.png")));
    connect(swapStationPb, &QPushButton::clicked, this, &MainWindow::swapStation);

    hLayout->addWidget(label = new QLabel(tr("到达站: ")));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hLayout->addWidget(toStationLe = new CompleteEdit);
    toStationLe->setPlaceholderText(_("简拼/全拼/汉字"));
    toStationLe->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    toStationLe->setMaximumWidth(120);
    connect(toStationLe, &CompleteEdit::editingFinished, this, &MainWindow::userEndStationChanged);

    hLayout->addWidget(new QLabel(tr("日期: ")));
    hLayout->addWidget(tourDateDe = new QDateEdit);
    tourDateDe->setMinimumWidth(105);

    tourDateDe->setCalendarPopup(true);
    connect(tourDateDe, &QDateEdit::dateChanged, this, &MainWindow::userTourDateChanged);
    hLayout->addStretch();

    hLayout->addWidget(queryTicketPb = new QPushButton);
    queryTicketPb->setText(tr("查询"));
    connect(queryTicketPb, &QPushButton::clicked, this, &MainWindow::queryTicket);
    hLayout->addWidget(switchTicketShowTypePb = new QPushButton);
    switchTicketShowTypePb->setText(tr("显示票价"));
    connect(switchTicketShowTypePb, &QPushButton::clicked, this, &MainWindow::switchTicketShowType);
    hLayout->addWidget(pb = new QPushButton);
    pb->setText(tr("设置"));
    settingDialog = new SettingDialog(this);
    settingDialog->setUp();
    connect(pb, &QPushButton::clicked, settingDialog, &SettingDialog::show);
    hLayout->addWidget(grabTicketPb = new QPushButton);
    grabTicketPb->setText(tr("开始"));
    connect(grabTicketPb, &QPushButton::clicked, this, &MainWindow::startOrStopGrabTicket);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout);
    setUpTableView();
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->setSpacing(10);
    hLayout2->addWidget(tableView);
    vLayout->addLayout(hLayout2);

    passengerDialog = new PassengerDialog(this);
    passengerDialog->setUp();
    trainNoDialog = new TrainNoDialog(this);
    trainNoDialog->setUp(static_cast<QStandardItemModel *>(tableView->model()));
    seatTypeDialog = new SeatTypeDialog(this);
    seatTypeDialog->setUp();

    seatDialog = new SeatDialog(this);

    loginDialog = new LoginDialog;
    loginDialog->setUp();

    doGrabTicketTimer = new QTimer;
    connect(doGrabTicketTimer, &QTimer::timeout, this, &MainWindow::doGrabTicket);
    updateProgressBarTimer = new QTimer;
    connect(updateProgressBarTimer, &QTimer::timeout, this, [=] {
        int incrVal = grabTicketInterval >> 3;
        nextRequestProgress->setValue(nextRequestProgress->value() + incrVal);
    });

    createDockWidget();

    widget->setLayout(vLayout);
    setCentralWidget(widget);

    createHistoryInfoDialog();

    createUiComponent();

    readSettings();

    //handleReply();
    setTabOrder(fromStationLe, toStationLe);
    statusBar()->showMessage(QStringLiteral("准备就绪"));
    resize(QSize(1200, 800));
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
    browser->setOpenExternalLinks(true);

    //browser->setMinimumSize(QSize(200, 80));
    QHBoxLayout *layout = new QHBoxLayout;
    QGridLayout *gLayout = new QGridLayout;
    QPushButton *pb = new QPushButton;
    selectedPassengerTipsLabel = new QLabel(tr("已选0/0"));
    pb->setText(tr("选择乘车人"));
    connect(pb, &QPushButton::clicked, passengerDialog, &PassengerDialog::show);
    gLayout->addWidget(pb, 0, 0, 1, 1);
    gLayout->addWidget(selectedPassengerTipsLabel, 0, 1, 1, 1);
    pb = new QPushButton;
    selectedTrainTipsLabel = new QLabel(tr("已选0/0"));
    pb->setText(tr("选择车次"));
    connect(pb, &QPushButton::clicked, trainNoDialog, &TrainNoDialog::show);
    gLayout->addWidget(pb, 1, 0, 1, 1);
    gLayout->addWidget(selectedTrainTipsLabel, 1, 1, 1, 1);
    pb = new QPushButton;
    selectedSeatTypeTipsLabel = new QLabel(tr("已选0/0"));
    pb->setText(tr("选择席别"));
    connect(pb, &QPushButton::clicked, seatTypeDialog, &SeatTypeDialog::show);
    gLayout->addWidget(pb, 2, 0, 1, 1);
    gLayout->addWidget(selectedSeatTypeTipsLabel, 2, 1, 1, 1);
    pb = new QPushButton;
    selectedSeatTipsLabel = new QLabel(tr("已选0/0"));
    pb->setText(tr("选座"));
    connect(pb, &QPushButton::clicked, seatDialog, &SeatDialog::show);
    gLayout->addWidget(pb, 3, 0, 1, 1);
    gLayout->addWidget(selectedSeatTipsLabel, 3, 1, 1, 1);

    layout->addLayout(gLayout);
    layout->addWidget(browser);
    widget->setLayout(layout);
    dock->setWidget(widget);
}

void MainWindow::createHistoryInfoDialog()
{
    historyInfoDialog = new QDialog(this);
    historyInfoBrower = new QTextBrowser;
    historyInfoBrower->setTextColor(QColor(0, 139, 139));
    historyInfoBrower->setOpenExternalLinks(true);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(historyInfoBrower);
    QHBoxLayout *hlayout = new QHBoxLayout;
    QPushButton *pb = new QPushButton(tr("清空"));
    connect(pb, &QPushButton::clicked, historyInfoBrower, &QTextBrowser::clear);
    hlayout->addStretch();
    hlayout->addWidget(pb);
    hlayout->addStretch();
    vlayout->addLayout(hlayout);
    historyInfoDialog->setLayout(vlayout);
}

void MainWindow::createUiComponent()
{
    QAction *action;
    QMenu *menu;

    menu = menuBar()->addMenu(tr("&文件"));

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

    action = new QAction(tr("&退出..."), this);
    action->setShortcut(tr("Ctrl+Q"));
    action->setStatusTip(tr("退出程序"));
    connect(action, &QAction::triggered, this, &MainWindow::close);
    menu->addAction(action);

    menu = menuBar()->addMenu(tr("&显示"));

    action = new QAction(tr("&历史提交..."), this);
    action->setShortcut(tr("Ctrl+H"));
    action->setStatusTip(tr("查看历史提交记录"));
    connect(action, &QAction::triggered, historyInfoDialog, &QDialog::show);
    menu->addAction(action);

    action = new QAction(tr("&设置..."), this);
    action->setShortcut(tr("Ctrl+S"));
    action->setStatusTip(tr("打开设置界面"));
    connect(action, &QAction::triggered, settingDialog, &SettingDialog::show);
    menu->addAction(action);

    menu = menuBar()->addMenu(tr("&帮助"));

    action = new QAction(tr("关于..."), this);
    action->setShortcut(QKeySequence::HelpContents);
    action->setStatusTip(tr("显示版本信息"));
    connect(action, &QAction::triggered, this, &MainWindow::about);
    menu->addAction(action);

    action = new QAction(tr("关于&Qt..."), this);
    action->setShortcut(tr("F2"));
    action->setStatusTip(tr("显示Qt版本信息"));
    connect(action, &QAction::triggered, qApp, &QApplication::aboutQt);
    menu->addAction(action);

    initStatusBars();
}

void MainWindow::userStartStationChanged()
{
    UserData *ud = UserData::instance();
    QString staFromStation = fromStationLe->text().trimmed();
    ud->getUserConfig().staFromName = staFromStation;
    ud->getUserConfig().staFromCode = ud->getStaCode()->value(staFromStation);
    trainNoDialog->clearUnSelectedTrain();
    QSettings setting;
    setting.setValue(_("query_ticket/from_station_name"), staFromStation);
}

void MainWindow::userEndStationChanged()
{
    UserData *ud = UserData::instance();
    QString staToStation = toStationLe->text().trimmed();
    ud->getUserConfig().staToName = staToStation;
    ud->getUserConfig().staToCode = ud->getStaCode()->value(staToStation);
    trainNoDialog->clearUnSelectedTrain();
    QSettings setting;
    setting.setValue(_("query_ticket/to_station_name"), staToStation);
}

void MainWindow::userTourDateChanged(const QDate &date)
{
    QString strDate = date.toString(QStringLiteral("yyyy-MM-dd"));
    UserData::instance()->getUserConfig().tourDate = strDate;
    trainNoDialog->clearUnSelectedTrain();
    QSettings setting;
    setting.setValue(_("query_ticket/tour_date"), strDate);
}

void MainWindow::swapStation()
{
    QString tmp;
    UserData *ud = UserData::instance();
    UserConfig &uc = ud->getUserConfig();

    tmp = fromStationLe->text();
    fromStationLe->setText(toStationLe->text());
    toStationLe->setText(tmp);

    uc.staFromName.swap(uc.staToName);
    uc.staFromCode.swap(uc.staToCode);
    trainNoDialog->clearUnSelectedTrain();
}

void MainWindow::uamLogined()
{
    statusBar()->showMessage(QStringLiteral("已登陆"));
    if (loginDialog)
        loginDialog->hide();
    show();
}

void MainWindow::uamNotLogined()
{
    statusBar()->showMessage(QStringLiteral("未登陆"));
    hide();
    if (loginDialog)
        loginDialog->show();
}

void MainWindow::setRemainTicketColor(QString &remain, QStandardItem *item)
{
    if (remain.isEmpty())
        return;
    if (!remain.compare(QStringLiteral("有")))
        item->setForeground(QBrush(QColor(144, 238, 144)));
    else if (!remain.compare(QStringLiteral("无")))
        item->setForeground(QBrush(QColor(190, 190, 190)));
    else if (!remain.compare(_("--")))
        item->setForeground(Qt::black);
    else
        item->setForeground(QBrush(QColor(238, 154, 73)));
}

void MainWindow::addTrainToSelected()
{
    QPushButton *button = static_cast<QPushButton *>(sender());
    //QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(tableView->model());
    //QStandardItem *item = itemModel->item(idx);
    //NetHelper::instance()->checkUser();
    trainNoDialog->addSelectedTrain(_("%1 (%2 %3").arg(button->property("trainCode").toString(),
                                                    button->property("fromStationName").toString(),
                                                    button->property("toStationName").toString()));
}

void MainWindow::processQueryTicketReply(QVariantMap &data)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(tableView->model());
    UserData *ud = UserData::instance();
    UserConfig &uc = ud->getUserConfig();
    QString staFromName = uc.staFromName;
    QString staToName = uc.staToName;
    QString tourDate = uc.tourDate;
    QVariantMap stationMap = data[_("map")].toMap();

    if (stationMap.isEmpty()) {
        //formatOutput(_("查询失败，返回站点信息为空"));
        formatOutput(_("%1->%2(%3) 共查询到 0 趟车次, 可预订 0 趟车次")
                         .arg(staFromName, staToName, tourDate));
        model->removeRows(0, model->rowCount());
        return;
    }
    QVariantList resultList = data[_("result")].toList();
    if (resultList.isEmpty()) {
        //formatOutput(_("查询失败，返回结果为空"));
        formatOutput(_("%1->%2(%3) 共查询到 0 趟车次, 可预订 0 趟车次")
                         .arg(staFromName, staToName, tourDate));
        model->removeRows(0, model->rowCount());
        return;
    }
    int trainListSize = resultList.size();
    int can_booking = 0;
    int i, itemIdx;
    int useTrainListSize = trainListSize;

    model->setRowCount(trainListSize);
    QVector<QStringList> allTrain;
    QVector<QStringList> availableTrain;
    QVector<QStringList> invalidTrain;
    QString fromStationName, toStationName;
    const QVector<QPair<int, int>> tableSeatTypeData = {
             QPair<int, int>(ESPECIALSEATCOL, ESWZNUM),
             QPair<int, int>(EFIRSTPRISEATCOL, EZYNUM),
             QPair<int, int>(ESECONDPRISEATCOL, EZENUM),
             QPair<int, int>(EADVSOFTCROUCHCOL, EGRNUM),
             QPair<int, int>(ESOFTCROUCHCOL, ERWNUM),
             QPair<int, int>(ESTIRCROUCHCOL, EYBNUM),
             QPair<int, int>(EHARDCROUCHCOL, EYWNUM),
             QPair<int, int>(ESOFTSEATCOL, ERZNUM),
             QPair<int, int>(EHARDSEATCOL, EYZNUM),
             QPair<int, int>(ENOSEATCOL, EWZNUM),
             QPair<int, int>(EOTHERCOL, EQTNUM)
    };
    QVector<QStandardItem *> tableSeatTypeItems;
    QMap<char, QStandardItem *> tableSeatTypeItemsMap;

    // 抢票模式先分析车票再渲染，防止渲染浪费时间
    if (ud->runStatus == EGRABTICKET) {
        for (i = 0; i < trainListSize; i++) {
            QString train = resultList[i].toString();
            QStringList trainInfo = train.split('|');

            if (trainInfo.size() < ESEATTYPES) {
                continue;
            }
            if (trainInfo[ESTATIONTRAINCODE].isEmpty()) {
                continue;
            }
            if (trainInfo[ECANWEBBUY] == _("Y")) {
                if (ud->runStatus == EGRABTICKET) {
                    availableTrain.push_back(trainInfo);
                }
                can_booking++;
            }
            if (ud->candidateSetting.isCandidate) {
                allTrain.push_back(trainInfo);
            }
        }
        Analysis candidateAnalysis(allTrain);
        if (ud->candidateSetting.isCandidate &&
            ud->candidateSetting.onlyCandidate) {
            if (ud->candidateRunStatus == ESUBMITCANDIDATESUCCESS) {
                // 结束任务
                startOrStopGrabTicket();
            }
            candidateAnalysis.mayCandidate(stationMap);
        } else {
            if (ud->candidateSetting.isCandidate && ud->candidateSetting.prioCandidate) {
                candidateAnalysis.mayCandidate(stationMap);
            }
            if (can_booking) {
                Analysis ana(availableTrain);
                std::pair<QString, QString> ticketStr;
                QVector<QChar> submitSeatType;
                int trainNoIdx = ana.analysisTrain(ticketStr, submitSeatType, stationMap);
                if (trainNoIdx != -1) {
                    ud->submitTicketInfo.trainCode = availableTrain[trainNoIdx][ESTATIONTRAINCODE];
                    ud->submitTicketInfo.trainNo = availableTrain[trainNoIdx][ETRAINNO];
                    ud->submitTicketInfo.secretStr = availableTrain[trainNoIdx][ESECRETSTR];
                    ud->submitTicketInfo.ypDetailInfo = availableTrain[trainNoIdx][EYPINFO];
                    ud->submitTicketInfo.date = tourDate;
                    ud->submitTicketInfo.fromStationCode = availableTrain[trainNoIdx][EFROMSTATIONTELECODE];
                    ud->submitTicketInfo.fromStationName = stationMap.value(availableTrain[trainNoIdx]
                                                                                          [EFROMSTATIONTELECODE])
                                                               .toString();
                    ud->submitTicketInfo.toStationCode = availableTrain[trainNoIdx][ETOSTATIONTELECODE];
                    ud->submitTicketInfo.toStationName = stationMap.value(availableTrain[trainNoIdx]
                                                                                        [ETOSTATIONTELECODE])
                                                             .toString();
                    ud->submitTicketInfo.submitSeatType = submitSeatType;
                    ud->submitTicketInfo.passengerTicketInfo = ticketStr.first;
                    ud->submitTicketInfo.oldPassengerTicketInfo = ticketStr.second;
                    NetHelper::instance()->checkUser();
                }
            }
            if (ud->candidateSetting.isCandidate && !ud->candidateSetting.prioCandidate) {
                candidateAnalysis.mayCandidate(stationMap);
            }
        }
    }

    itemIdx = 0;
    // 开始显示到tableview
    for (i = 0; i < trainListSize; i++) {
        QString train = resultList[i].toString();
        QStringList trainInfo = train.split('|');
        QStandardItem  *item;
        QPushButton *button;
        QString curText;

        if (trainInfo.size() < ESEATTYPES) {
            useTrainListSize--;
            continue;
        }
        if (trainInfo[ESTATIONTRAINCODE].isEmpty()) {
            useTrainListSize--;
            continue;
        }
        QStringList dwFlag = trainInfo[EDWFLAG].split('#');
        bool isFunXing = dwFlag.length() > 1 && dwFlag[1] == '1';  // 复兴号
        bool isZiNeng = !dwFlag.isEmpty() && dwFlag[0] == '5';  // 智能列车
        bool isDongGan = dwFlag.length() > 5 && dwFlag[5] == 'D';  // 动感号

        char trainCodePrefix = trainInfo[ESTATIONTRAINCODE][0].toLatin1();
        if ((settingDialog->trainTypeShowVec['5'] && isZiNeng) ||
            (settingDialog->trainTypeShowVec['1'] && isFunXing) ||
            (settingDialog->trainTypeShowVec['d'] && isDongGan)) {

        } else {
            if (settingDialog->trainTypeShowVec[trainCodePrefix] == false) {
                if (settingDialog->trainTypeShowVec['O'] == false) {
                    useTrainListSize--;
                    continue;
                }
            }
        }

        fromStationName = stationMap.value(trainInfo[EFROMSTATIONTELECODE]).toString();
        toStationName = stationMap.value(trainInfo[ETOSTATIONTELECODE]).toString();

        if (ud->runStatus != EGRABTICKET) {
            if (trainInfo[ECANWEBBUY] == _("Y")) {
                can_booking++;
            }
        }
        if (trainInfo[ECANWEBBUY] == _("IS_TIME_NOT_BUY")) {
            invalidTrain.push_back(trainInfo);
            useTrainListSize--;
            continue;
        }

        //QWidget *trainWidget = new QWidget;
        //QHBoxLayout *hlayout=new QHBoxLayout;
        //QCheckBox *trainCBox = new QCheckBox;
        //trainCBox->setObjectName("singleCheck");
        //QLabel *label = new QLabel(trainInfo[ESTATIONTRAINCODE]);
        //item = new QStandardItem();
        //hlayout->addWidget(trainCBox);
        //hlayout->addWidget(label);
        //model->setItem(i, ETRAINNUM, );
        //tableView->setIndexWidget(model->index(i, 0), trainWidget);

        ud->submitTicketInfo.secretStr = trainInfo[ESECRETSTR];

        curText = trainInfo[ESTATIONTRAINCODE].isEmpty() ?
                      "--" : trainInfo[ESTATIONTRAINCODE];
        item = model->item(itemIdx, ETRAINNOCOL);
        if (!item) {
            model->setItem(itemIdx, ETRAINNOCOL, item = new QStandardItem(curText));
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(QColor(99, 184, 255)));
        }
        if (item->text() != curText) {
            item->setText(curText);
            item->setToolTip(curText);
        }
        item->setData(trainInfo[ETRAINNO], Qt::UserRole);

/*
        if (isFunXing) {
            QIcon icon;
            QPixmap pixmap;
            pixmap.load(_(":/icon/images/ticket.ico"));
            icon.addPixmap(pixmap);
            item->setData(icon, Qt::DecorationRole);
            item->setData(trainInfo[ESTATIONTRAINCODE], Qt::DisplayRole);
        } else if (isZiNeng) {
            //QIcon icon(_(":/icon/images/zineng.png"));
            QIcon icon;
            QPixmap pixmap;
            pixmap.load(_(":/icon/images/ticket.ico"));
            icon.addPixmap(pixmap);
            item->setData(icon, Qt::DecorationRole);
            item->setData(trainInfo[ESTATIONTRAINCODE], Qt::DisplayRole);
        }
*/

        item = model->item(itemIdx, EFROMSTATIONCOL);
        if (!item) {
            model->setItem(itemIdx, EFROMSTATIONCOL, item = new QStandardItem);
        }
        if (item->text() != fromStationName) {
            item->setText(fromStationName);
            item->setToolTip(fromStationName);
            item->setData(fromStationName, Qt::ToolTipRole);
        }
        //item->setTextAlignment(Qt::AlignCenter);
        /*item->setFont(QFont("Times", 10, QFont::Black));
        item->setForeground(QBrush(QColor(99, 184, 255)));*/
        item->setData(trainInfo[EFROMSTATIONTELECODE] == trainInfo[ESTARTSTATIONTELECODE], Qt::DecorationRole);
        item->setData(trainInfo[EFROMSTATIONTELECODE], Qt::UserRole);


        item = model->item(itemIdx, ETOSTATIONCOL);
        if (!item) {
            model->setItem(itemIdx, ETOSTATIONCOL, item = new QStandardItem);
            item->setTextAlignment(Qt::AlignCenter);
            item->setToolTip(toStationName);
            item->setFont(QFont("Times", 10, QFont::Black));
            item->setForeground(QBrush(QColor(99, 184, 255)));
        }
        if (item->text() != toStationName) {
            item->setText(toStationName);
            item->setToolTip(toStationName);
            item->setData(toStationName, Qt::ToolTipRole);
        }
        item->setData(trainInfo[ETOSTATIONTELECODE] == trainInfo[EENDSTATIONTELECODE], Qt::DecorationRole);
        item->setData(trainInfo[ETOSTATIONTELECODE], Qt::UserRole);

        item = model->item(itemIdx, ESTARTTIMECOL);
        if (!item) {
            model->setItem(itemIdx, ESTARTTIMECOL, item = new QStandardItem(trainInfo[ESTARTTIME]));
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(QColor(205, 104, 137)));

        }
        if (item->text() != trainInfo[ESTARTTIME]) {
            item->setText(trainInfo[ESTARTTIME]);
        }

        item = model->item(itemIdx, EARRIVETIMECOL);
        if (!item) {
            model->setItem(itemIdx, EARRIVETIMECOL, item = new QStandardItem(trainInfo[EARRIVETIME]));
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(QColor(205, 104, 137)));

        }
        if (item->text() != trainInfo[EARRIVETIME]) {
            item->setText(trainInfo[EARRIVETIME]);
        }

        item = model->item(itemIdx, EUSEDTIMECOL);
        if (!item) {
            model->setItem(itemIdx, EUSEDTIMECOL, item = new QStandardItem(trainInfo[ESPENDTIME]));
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(QColor(122, 139, 139)));
        }
        if (item->text() != trainInfo[ESPENDTIME]) {
            item->setText(trainInfo[ESPENDTIME]);
        }

        for (auto &seatTypeData : tableSeatTypeData) {
            item = model->item(itemIdx, seatTypeData.first);
            if (!item) {
                item = new QStandardItem;
                model->setItem(itemIdx, seatTypeData.first, item);
            }

            if (uc.showTicketPrice == false) {
                curText = trainInfo[seatTypeData.second].isEmpty() ? _("--") :
                                  trainInfo[seatTypeData.second];
                if (item->text() != curText) {
                    item->setText(curText);
                    setRemainTicketColor(curText, item);
                    item->setTextAlignment(Qt::AlignCenter);
                }
            }

            if (seatTypeData.first == EOTHERCOL) {
                int type = 0;
                if (isFunXing) {
                    type = 1 << 0;
                }
                if (isZiNeng) {
                    type = type | (1 << 1);
                }
                if (isDongGan) {
                    type = type | (1 << 2);
                }
                item->setData(type, Qt::UserRole);
            }

            tableSeatTypeItems.push_back(item);
        }


        button = dynamic_cast<QPushButton *>(tableView->indexWidget(model->index(itemIdx, EREMARKCOL)));
        if (!button) {
            button = new QPushButton(tr("添加"));
            if (ud->runStatus == EGRABTICKET) {
                //button->setStyleSheet(QStringLiteral("QPushButton { background-color: #A9A9A9; color: #4F4F4F; }"));
                button->setEnabled(false);
            } else {
                //button->setStyleSheet(QStringLiteral("QPushButton { color: #1C86EE; }"));
                button->setProperty("fromStationName", fromStationName);
                button->setProperty("toStationName", toStationName);
                button->setProperty("trainCode", trainInfo[ESTATIONTRAINCODE]);
                connect(button, &QPushButton::clicked, this, &MainWindow::addTrainToSelected);
            }
        } else {
            button->setProperty("fromStationName", fromStationName);
            button->setProperty("toStationName", toStationName);
            button->setProperty("trainCode", trainInfo[ESTATIONTRAINCODE]);
        }
        tableView->setIndexWidget(model->index(itemIdx, EREMARKCOL), button);

        // 席别余票分析，显示余票
        if (ud->runStatus == EIDLE) {
            QString &ypInfoNew = trainInfo[EYPINFONEW];
            int ypInfoNewSize = ypInfoNew.size();

            tableSeatTypeItemsMap.insert('9', tableSeatTypeItems[0]);
            tableSeatTypeItemsMap.insert('P', tableSeatTypeItems[0]);
            tableSeatTypeItemsMap.insert('M', tableSeatTypeItems[1]);
            tableSeatTypeItemsMap.insert('O', tableSeatTypeItems[2]);
            tableSeatTypeItemsMap.insert('S', tableSeatTypeItems[2]);
            tableSeatTypeItemsMap.insert('6', tableSeatTypeItems[3]);
            tableSeatTypeItemsMap.insert('A', tableSeatTypeItems[3]);
            tableSeatTypeItemsMap.insert('4', tableSeatTypeItems[4]);
            tableSeatTypeItemsMap.insert('I', tableSeatTypeItems[4]);
            tableSeatTypeItemsMap.insert('F', tableSeatTypeItems[5]);
            tableSeatTypeItemsMap.insert('3', tableSeatTypeItems[6]);
            tableSeatTypeItemsMap.insert('J', tableSeatTypeItems[6]);
            tableSeatTypeItemsMap.insert('2', tableSeatTypeItems[7]);
            tableSeatTypeItemsMap.insert('1', tableSeatTypeItems[8]);

            for (auto &seatTypeData : tableSeatTypeData) {
                tableSeatTypeItems[seatTypeData.first - ESPECIALSEATCOL]->setData(
                    0, Qt::StatusTipRole);
            }
            for (auto &seatTypeData : tableSeatTypeData) {
                tableSeatTypeItems[seatTypeData.first - ESPECIALSEATCOL]->setData(
                    trainInfo[seatTypeData.second].isEmpty() ? _("--") :
                        trainInfo[seatTypeData.second], Qt::StatusTipRole);
            }
            for (int j = 0; j < ypInfoNewSize; j += 10) {
                if (j + 10 > ypInfoNewSize) {
                    break;
                }
                int price = ypInfoNew.sliced(j + 1, 5).toInt();
                float price2 = price / 10.0;
                int dd = ypInfoNew.sliced(j + 6, 4).toInt();
                char t = ypInfoNew[j].toLatin1();
                QMap<char, QStandardItem *>::ConstIterator it =
                    tableSeatTypeItemsMap.find(t);
                if (it != tableSeatTypeItemsMap.cend()) {
                    it.value()->setToolTip(_("%1").arg(price2));
                    it.value()->setData(price2, Qt::ToolTipRole);
                    if (uc.showTicketPrice) {
                        it.value()->setText(_("%1").arg(price2));
                    }
                } else {
                    // 其他
                    if (dd < 3000) {
                        if (tableSeatTypeItems.size() > 10) {
                            tableSeatTypeItems[10]->setToolTip(_("%1").arg(price2));
                            tableSeatTypeItems[10]->setData(price2, Qt::ToolTipRole);
                        }
                    }
                }
                // 无座
                if (dd >= 3000) {
                    tableSeatTypeItems[9]->setToolTip(_("%1").arg(price2));
                    tableSeatTypeItems[9]->setData(price2, Qt::ToolTipRole);
                }
            }


            tableSeatTypeItemsMap.clear();
        }
        trainNoDialog->addTrain(_("%1%2%3").arg(trainInfo[ESTATIONTRAINCODE],
                                                trainInfo[EFROMSTATIONTELECODE],
                                                trainInfo[ETOSTATIONTELECODE]),
                                _("%1 (%2 %3 %4-%5 %6)").arg(trainInfo[ESTATIONTRAINCODE],
                                                                fromStationName,
                                                                toStationName,
                                                                trainInfo[ESTARTTIME],
                                                                trainInfo[EARRIVETIME],
                                                                trainInfo[ESPENDTIME]));
        tableSeatTypeItems.clear();
        itemIdx++;
    }
    model->setRowCount(useTrainListSize);
    trainNoDialog->addTrainFinish();
    // 列车运行图调整的列车
    int invalidTrainSize = invalidTrain.size();
    for (i = 0; i < invalidTrainSize; i++) {
        QStringList &trainInfo = invalidTrain[i];
        QStandardItem *item;
        model->setItem(itemIdx, ETRAINNOCOL, item = new QStandardItem(trainInfo[ESTATIONTRAINCODE].isEmpty() ?
                                                                    "--" : trainInfo[ESTATIONTRAINCODE]));
        item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(trainInfo[ESTATIONTRAINCODE]);
        fromStationName = stationMap.value(trainInfo[EFROMSTATIONTELECODE]).toString();
        toStationName = stationMap.value(trainInfo[ETOSTATIONTELECODE]).toString();
        model->setItem(itemIdx, EFROMSTATIONCOL, item = new QStandardItem);
        //item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(fromStationName);
        model->setItem(itemIdx, ETOSTATIONCOL, item = new QStandardItem);
        //item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(toStationName);
        for (int k = ESTARTTIMECOL; k < EREMARKCOL; k++) {
            model->setItem(itemIdx, k, item = new QStandardItem(_("--")));
            item->setTextAlignment(Qt::AlignCenter);
        }
        model->setItem(itemIdx, EREMARKCOL, item = new QStandardItem(trainInfo[ETEXTINFO]));
        item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(trainInfo[ETEXTINFO]);
        itemIdx++;
    }
    //tableView->resizeColumnToContents(EFROMSTATIONCOL);
    //tableView->resizeColumnToContents(ETOSTATIONCOL);

    if (useTrainListSize == trainListSize) {
        formatOutput(_("%1->%2(%3) 共查询到 %4 趟车次, 可预订 %5 趟车次").
                     arg(staFromName, staToName, tourDate).arg(trainListSize).arg(can_booking));
    } else {
        formatOutput(_("%1->%2(%3) 共查询到 %4 趟车次, 已过滤 %5 趟车次, 可预订 %6 趟车次").
                     arg(staFromName, staToName, tourDate).arg(trainListSize)
                     .arg(trainListSize - useTrainListSize).arg(can_booking));
    }
}

void MainWindow::setStationNameCompleter(const QByteArray &nameText)
{
    InputCompleter *ic, *ic2;
    UserData *ud = UserData::instance();
    UserConfig &uc = ud->getUserConfig();

    ic = new InputCompleter(this);
    ic->setStationData(nameText);
    ic->setCaseSensitivity((Qt::CaseInsensitive));
    fromStationLe->setCompleter(ic);
    ic->popup()->setStyleSheet("background-color: #F5F5F5;\
                               color: #000000;\
                               border: 1px solid #BEBEBE;\
                                                          border-radius: 5px;\
                               padding: 2px 2px 2px 2px;\
                                                         min-width: 17px;\
                               font: 14px \"Arial\";");
    if (!uc.staFromName.isEmpty() && uc.staFromCode.isEmpty()) {
        uc.staFromCode = ud->getStaCode()->value(uc.staFromName);
    }

    ic2 = new InputCompleter(ic);
    *ic2 = *ic;
    ic2->setCaseSensitivity((Qt::CaseInsensitive));
    toStationLe->setCompleter(ic2);
    ic2->popup()->setStyleSheet("background-color: #F5F5F5;\
                               color: #000000;\
                               border: 1px solid #BEBEBE;\
                                                          border-radius: 5px;\
                               padding: 2px 2px 2px 2px;\
                                                         min-width: 17px;\
                               font: 14px \"Arial\";");
    if (!uc.staToName.isEmpty() && uc.staToCode.isEmpty()) {
        uc.staToCode = ud->getStaCode()->value(uc.staToName);
    }
}

void MainWindow::queryTicket()
{
    UserData *ud = UserData::instance();
    UserConfig &uc = ud->getUserConfig();

    if (uc.staFromCode.isEmpty()) {
        formatOutput(_("请先填写出发站！"));
        return;
    }
    if (uc.staToName.isEmpty()) {
        formatOutput(_("请先填写到达站！"));
        return;
    }

    if (!uc.staFromCode.isEmpty() && !uc.staToCode.isEmpty()) {
        if (ud->runStatus == EGRABTICKET || ud->runStatus == EIDLE) {
            NetHelper::instance()->queryTicket();
        }
    } else {
        formatOutput(_("出发站或到达站的站点识别码为空！"));
    }
}

void MainWindow::queryTrainStopStation()
{
    QItemSelectionModel *modelSelection = tableView->selectionModel();
    QModelIndexList indexList= modelSelection->selectedRows();
    QList<int> columns = {
        ETRAINNOCOL,
        EFROMSTATIONCOL,
        ETOSTATIONCOL
    };
    QList<QString> args;
    UserData *ud = UserData::instance();

    if (!indexList.isEmpty()) {
        const QStandardItemModel *itemModel;
        const QStandardItem *item;
        for (auto &col : columns) {
            itemModel = dynamic_cast<const QStandardItemModel *>(indexList[0].model());
            item = dynamic_cast<const QStandardItem *>(itemModel->item(indexList[0].row(), col));
            QString data = item->data(Qt::UserRole).toString();
            if (!data.isEmpty()) {
                args.push_back(data);
            }
        }

        if (args.size() == columns.size()) {
            if (!ud->getUserConfig().tourDate.isEmpty()) {
                args.push_back(ud->getUserConfig().tourDate);
                NetHelper::instance()->queryTrainStopStation(args);
            }
        }
    }
}

void MainWindow::processStopStationReply(QVariantMap &data)
{
    QVariantMap data1 = data[_("data")].toMap();
    if (!data.isEmpty()) {
        QStringList headers = {
            _("站序"),
            _("站名"),
            _("到站时间"),
            _("出站时间"),
            _("停靠时间")
        };
        QStringList columnsData = {
                               _("station_no"),
            _("station_name"),
            _("arrive_time"),
            _("start_time"),
            _("stopover_time")
        };
        QString disp;
        QVariantList data2 = data1[_("data")].toList();
        QTableWidget *tw = new QTableWidget(data2.length(), headers.size());
        tw->setHorizontalHeaderLabels(headers);
        tw->setSelectionMode(QAbstractItemView::NoSelection);
        tw->setEditTriggers(QAbstractItemView::NoEditTriggers);

        for (int i = 0; i < data2.length(); i++) {
            QVariantMap data3 = data2[i].toMap();
            QString s;
            /*
            if (i == 0) {
                s = data3[_("service_type")].toString();
                QTableWidgetItem *item = new QTableWidgetItem(tr(s == _("0") ? _("")))
            }*/
            if (i == 0) {
                s = data3[_("station_train_code")].toString();
                disp += s + _("  ");
                s = data3[_("start_station_name")].toString();
                disp += s + _("->");
                s = data3[_("end_station_name")].toString();
                disp += s + _("  ");
                s = data3[_("train_class_name")].toString();
                disp += s == '0' ? _("无空调") : _("有空调");
            }
            int j = 0;
            for (auto &column : columnsData) {
                s = data3[column].toString();
                QTableWidgetItem *item = new QTableWidgetItem(s);
                tw->setItem(i, j, item);
                j++;
            }
        }
        if (tw->rowCount()) {
            QDialog m;
            QLabel l(disp);
            //scrollArea.viewport()->setBackgroundRole(QPalette::);
            QVBoxLayout vlayout;
            tw->resizeColumnsToContents();
            vlayout.addWidget(&l);
            vlayout.addWidget(tw);
            m.setLayout(&vlayout);
            m.setWindowTitle(_("停靠站信息"));
            m.resize(350, 250);
            m.exec();
            delete tw;
        }
    }
}

void MainWindow::prepareGrabTicket(bool status)
{
    UserData *ud = UserData::instance();
    UserConfig &uc = ud->getUserConfig();

    if (status) {
        UserData::instance()->setRunStatus(EGRABTICKET);
        UserData::instance()->candidateRunStatus = EGRABCANDIDATETICKET;
        grabTicketPb->setText(tr("停止"));
        fromStationLe->setEnabled(false);
        toStationLe->setEnabled(false);
        swapStationPb->setEnabled(false);
        tourDateDe->setEnabled(false);

        doGrabTicketTimer->setInterval(1000);
        doGrabTicketTimer->start();
        updateProgressBarTimer->start(100);
        grabTicketInterval = 1000;
        if (uc.showTicketPrice) {
            switchTicketShowType();
        }
        queryTicketPb->setEnabled(false);
        switchTicketShowTypePb->setEnabled(false);
        passengerDialog->enterGrabTicketMode();
        seatTypeDialog->enterGrabTicketMode();
        trainNoDialog->enterGrabTicketMode();
        tableView->setContextMenuPolicy(Qt::NoContextMenu);
    } else {
        UserData::instance()->setRunStatus(EIDLE);
        UserData::instance()->candidateRunStatus = EIDLE;
        grabTicketPb->setText(tr("开始"));
        fromStationLe->setEnabled(true);
        toStationLe->setEnabled(true);
        swapStationPb->setEnabled(true);
        tourDateDe->setEnabled(true);

        doGrabTicketTimer->stop();
        updateProgressBarTimer->stop();
        grabTicketInterval = 0;

        queryTicketPb->setEnabled(true);
        switchTicketShowTypePb->setEnabled(true);
        passengerDialog->exitGrabTicketMode();
        seatTypeDialog->exitGrabTicketMode();
        trainNoDialog->exitGrabTicketMode();
        tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    }
}

void MainWindow::startOrStopGrabTicket()
{
    static bool grabTicketPbStatus;

    UserData *ud = UserData::instance();
    UserConfig &uc = ud->getUserConfig();

    if (!grabTicketPbStatus) {
        if (uc.staFromCode.isEmpty()) {
            formatOutput(_("请先填写出发站！"));
            return;
        }
        if (uc.staToName.isEmpty()) {
            formatOutput(_("请先填写到达站！"));
        }
        const QStringList &selectedPassengerList = passengerDialog->getSelectedPassenger();
        if (selectedPassengerList.isEmpty()) {
            formatOutput(_("请先选择乘车人！"));
            return;
        }
        const QStringList &selectedTrainList = trainNoDialog->getSelectedTrainList();
        if (selectedTrainList.isEmpty()) {
            formatOutput(_("请先选择车次！"));
            return;
        }
        const QStringList &selectedSeatTypeList = seatTypeDialog->getSelectedSeatType();
        if (selectedSeatTypeList.isEmpty()) {
            formatOutput(_("请先选择席别！"));
            return;
        }
    }
    grabTicketPbStatus = !grabTicketPbStatus;
    prepareGrabTicket(grabTicketPbStatus);
}

void MainWindow::doGrabTicket()
{
    std::uniform_int_distribution<int> dist(2000, 6000);
    if (UserData::instance()->runStatus == EGRABTICKET) {
        grabTicketInterval = dist(*QRandomGenerator::global());
        doGrabTicketTimer->setInterval(grabTicketInterval);
        updateProgressBarTimer->setInterval(grabTicketInterval >> 3);
        nextRequestProgress->setRange(0, grabTicketInterval);
        nextRequestProgress->reset();
        queryTicket();
    }
}

void MainWindow::switchTableTicketShowType(bool showTicketPrice)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(tableView->model());
    int rowcount = model->rowCount();
    QVariant data;
    QString text;
    QStandardItem *item;
    const QVector<int> tableSeatTypeData = {
        ESPECIALSEATCOL,
        EFIRSTPRISEATCOL,
        ESECONDPRISEATCOL,
        EADVSOFTCROUCHCOL,
        ESOFTCROUCHCOL,
        ESTIRCROUCHCOL,
        EHARDCROUCHCOL,
        ESOFTSEATCOL,
        EHARDSEATCOL,
        ENOSEATCOL,
        EOTHERCOL
    };
    int role;
    if (showTicketPrice) {
        role = Qt::ToolTipRole;
    } else {
        role = Qt::StatusTipRole;
    }
    for (int i = 0; i < rowcount; i++) {
        for (auto &seatTypeData : tableSeatTypeData) {
            data = model->index(i, seatTypeData).data(role);
            item = model->item(i, seatTypeData);

            if (data.isValid()) {
                text = data.toString();
                if (text != '0') {
                    item->setText(text);
                    if (showTicketPrice) {
                        item->setForeground(QBrush(QColor(238, 118, 33)));
                    } else {
                        setRemainTicketColor(text, item);
                    }
                } else {
                    item->setText(_("--"));
                    item->setForeground(Qt::black);
                }
            } else {
                item->setText(_("--"));
                item->setForeground(Qt::black);
            }
        }
    }
}

void MainWindow::switchTicketShowType()
{
    UserConfig &uc = UserData::instance()->getUserConfig();
    uc.showTicketPrice = !uc.showTicketPrice;
    if (uc.showTicketPrice) {
        switchTicketShowTypePb->setText(tr("显示余票"));
    } else {
        switchTicketShowTypePb->setText(tr("显示票价"));
    }
    switchTableTicketShowType(uc.showTicketPrice);
}

void MainWindow::loadStationName()
{
    QFile file(QStringLiteral("./station_name_" STATIONNAMEVERSION ".txt"));

    if (file.open(QIODevice::ReadOnly)) {
        //QTextStream nameFile(&file);
        //QString nameText = nameFile.readAll();
        setStationNameCompleter(file.readAll());
        file.close();
        return;
    }
    NetHelper::instance()->getStationNameTxt();
}

void MainWindow::canCandidate()
{

}

void MainWindow::chooseSeat()
{

}

void MainWindow::rightMenuSelectTrain()
{
    QItemSelectionModel *modelSelection = tableView->selectionModel();
    QModelIndexList indexList= modelSelection->selectedRows();

    foreach (QModelIndex index, indexList) {
        const QStandardItemModel *itemModel = dynamic_cast<const QStandardItemModel *>(index.model());
        const QStandardItem *item = dynamic_cast<const QStandardItem *>(itemModel->item(index.row(), 0));
        trainNoDialog->addSelectedTrain(item->text());
    }
}

void MainWindow::rightMenuUnselectTrain()
{
    QItemSelectionModel *modelSelection = tableView->selectionModel();
    QModelIndexList indexList= modelSelection->selectedRows();

    foreach (QModelIndex index, indexList) {
        const QStandardItemModel *itemModel = dynamic_cast<const QStandardItemModel *>(index.model());
        const QStandardItem *item = dynamic_cast<const QStandardItem *>(itemModel->item(index.row(), 0));
        trainNoDialog->removeSelectedTrain(item->text());
    }
}

void MainWindow::rightMenuSelectTrainAll()
{
    trainNoDialog->addSelectedTrainAll();
}

void MainWindow::rightMenuUnselectTrainAll()
{
    trainNoDialog->clearSelectedTrain();
}

void MainWindow::createRightMenu()
{
    rightMenu = new QMenu;
    QAction *select = new QAction("选中车次",this);
    QAction *unSelect = new QAction("取消选中车次",this);
    QAction *selectAll = new QAction("选中所有车次",this);
    QAction *unSelectAll = new QAction("取消选中所有车次",this);
    QAction *showStopStation = new QAction("显示停靠站信息",this);

    connect(select, &QAction::triggered, this, &MainWindow::rightMenuSelectTrain);
    connect(unSelect, &QAction::triggered, this, &MainWindow::rightMenuUnselectTrain);
    connect(selectAll, &QAction::triggered, this, &MainWindow::rightMenuSelectTrainAll);
    connect(unSelectAll, &QAction::triggered, this, &MainWindow::rightMenuUnselectTrainAll);
    connect(showStopStation, &QAction::triggered, this, &MainWindow::queryTrainStopStation);

    rightMenu->addAction(select);
    rightMenu->addAction(unSelect);
    rightMenu->addAction(selectAll);
    rightMenu->addAction(unSelectAll);
    rightMenu->addAction(showStopStation);
}

void MainWindow::setUpTableView()
{
    tableView = new QTableView;
    QStandardItemModel *model = new QStandardItemModel;
    tableView->verticalHeader()->setDefaultSectionSize(20);
    tableView->resize(tableView->sizeHint());
    //QCheckBox *checkBoxAll = new QCheckBox;
    //tableView->setCornerWidget(checkBoxAll);
    //tableView->setCornerButtonEnabled(true);
    Q_ASSERT(trainTableColumnDesc.size() == ETRAINTABLECOLUMNENDTOTAL);
    /*model->setColumnCount(ETRAINTABLECOLUMNENDTOTAL);
    for (size_t i = 0; i < ETRAINTABLECOLUMNENDTOTAL - 1; ++i) {
        model->setHeaderData(i, Qt::Horizontal, trainTableColumnDesc[i], Qt::DisplayRole);
    }
    model->setHeaderData(ETRAINTABLECOLUMNENDTOTAL - 1, Qt::Horizontal,
                         trainTableColumnDesc[ETRAINTABLECOLUMNENDTOTAL - 1], Qt::EditRole);*/
    model->setHorizontalHeaderLabels(trainTableColumnDesc);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
    IconDelegate *iconDelegate = new IconDelegate(this);
    tableView->setItemDelegate(iconDelegate);
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

void MainWindow::updateNetQualityStatus(int ms)
{
    static int lastStatus = -1;
    int status;
    QPixmap pixmap;

    if (ms != -1) {
        if (ms < 100) {
            status = 0;
            if (status != lastStatus) {
                lastStatus = status;
                pixmap.load(_(":/icon/images/perfect.png"));
                netQualityIndicateLabel->setPixmap(pixmap);
            }
            netQualityIndicateDescLabel->setText(tr("%1ms 非常好").arg(ms));
        } else if (ms < 500) {
            status = 1;
            if (status != lastStatus) {
                pixmap.load(_(":/icon/images/good.png"));
                netQualityIndicateLabel->setPixmap(pixmap);
            }
            netQualityIndicateDescLabel->setText(tr("%1ms 良好").arg(ms));
        } else if (ms < 1000) {
            status = 2;
            if (status != lastStatus) {
                pixmap.load(_(":/icon/images/good.png"));
                netQualityIndicateLabel->setPixmap(pixmap);
            }
            netQualityIndicateDescLabel->setText(tr("%1ms 差").arg(ms));
        } else {
            if (ms < 2000) {
                status = 3;
                netQualityIndicateDescLabel->setText(tr("%1ms 非常差").arg(ms));
            } else {
                status = 4;
                netQualityIndicateDescLabel->setText(tr("> 2000ms 非常差"));
            }
            if (status != lastStatus) {
                lastStatus = status;
                pixmap.load(_(":/icon/images/bad.png"));
                netQualityIndicateLabel->setPixmap(pixmap);

            }
        }
    } else {
        status = 5;
        if (status != lastStatus) {
            lastStatus = status;
            pixmap.load(_(":/icon/images/bad.png"));
            netQualityIndicateLabel->setPixmap(pixmap);
            netQualityIndicateDescLabel->setText(tr("中断"));
        }
    }
}

void MainWindow::readSettings()
{
    UserConfig &uc = UserData::instance()->getUserConfig();
    /*if (ud->readConfigFile()) {
        UserConfig &uc = ud->getUserConfig();
        fromStationLe->setText(uc.staFromName);
        uc.staFromCode = ud->getStaCode()->value(uc.staFromName);
        toStationLe->setText(uc.staToName);
        uc.staToCode = ud->getStaCode()->value(uc.staToName);
        tourDateDe->setDate(QDate::fromString(uc.tourDate, "yyyy-MM-dd"));
    }*/
    QSettings setting;
    QString text = setting.value(_("query_ticket/from_station_name"), _("")).value<QString>();
    fromStationLe->setText(text);
    uc.staFromName = text;
    uc.staFromCode = UserData::instance()->getStaCode()->value(text);
    text = setting.value(_("query_ticket/to_station_name"), _("")).value<QString>();
    uc.staToName = text;
    uc.staToCode = UserData::instance()->getStaCode()->value(text);
    toStationLe->setText(text);
    text = setting.value(_("query_ticket/tour_date"), _("")).value<QString>();
    QDate curDate = QDate::currentDate();
    QDate setDate = QDate::fromString(text, _("yyyy-MM-dd"));
    if (!setDate.isValid() || setDate < curDate) {
        tourDateDe->setDate(curDate);
    } else {
        tourDateDe->setDate(setDate);
    }
    tourDateDe->setDateRange(curDate, curDate.addDays(14));
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

void MainWindow::initStatusBars()
{
    /*QLabel *label = new QLabel;
    //label->setAlignment(Qt::AlignVCenter);
    //label->setMinimumSize(label->sizeHint());
    statusBar()->addWidget(label);

    label = new QLabel;
    label->setIndent(3);
    statusBar()->addWidget(label);*/
    nextRequestProgress = new QProgressBar;
    nextRequestProgress->setMaximumSize(100, 20);
    statusBar()->addPermanentWidget(nextRequestProgress);

    netQualityIndicateLabel = new QLabel;
    QPixmap pixmap;
    pixmap.load(_(":/icon/images/perfect.png"));
    netQualityIndicateLabel->setPixmap(pixmap);
    statusBar()->addPermanentWidget(netQualityIndicateLabel);

    netQualityIndicateDescLabel = new QLabel;
    netQualityIndicateDescLabel->setText(_("非常好"));
    statusBar()->addPermanentWidget(netQualityIndicateDescLabel);

}

void MainWindow::showStatusBarMessage(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainWindow::formatOutput(const QString &output)
{
    static QString textBuff;
    textBuff.clear();
    QDateTime date = QDateTime::currentDateTime();
    textBuff += date.toString(Qt::ISODate);
    textBuff += QStringLiteral(" ") + output;
    browser->append(textBuff);
    UserData *ud = UserData::instance();
    if (ud->runStatus == ESUBMITORDER ||
        ud->candidateRunStatus == ESUBMITCANDIDATE) {
        historyInfoBrower->append(textBuff);
    }
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
    delete tableView->itemDelegate();
    delete tableView;
}

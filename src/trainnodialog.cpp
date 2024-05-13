#include "trainnodialog.h"
#include "userdata.h"
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QTimeEdit>
#include <QSettings>
#include "mainwindow.h"

#define _ QStringLiteral

extern MainWindow *w;

TrainNoDialog::TrainNoDialog(QWidget *parent) :
    QDialog(parent)
{
}

TrainNoDialog::~TrainNoDialog()
{
}

void TrainNoDialog::setUp()
{
    unSelected = new QListWidget;
    unSelected->setMinimumWidth(300);
    selected = new QListWidget;
    selected->setMinimumWidth(300);
    QFont font;
    font.setPointSize(11);
    QLabel *label = new QLabel(tr("未选中车次："));
    label->setFont(font);

    unSelected->setCurrentRow(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(label);
    vLayout->addWidget(unSelected);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addStretch();
    addSelectedPb = new QPushButton;
    addSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_right.png")));
    connect(addSelectedPb, &QPushButton::clicked, this, &TrainNoDialog::setSelectedTrainNo);
    addSelectedPb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(addSelectedPb);

    addUnSelectedPb = new QPushButton;
    addUnSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_left.png")));
    connect(addUnSelectedPb, &QPushButton::clicked, this, &TrainNoDialog::setUnselectedTrainNo);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(addUnSelectedPb);
    vLayout2->addStretch();
    moveUpSelectedPb = new QPushButton;
    moveUpSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_up.png")));
    connect(moveUpSelectedPb, &QPushButton::clicked, this, &TrainNoDialog::moveUpTrain);
    vLayout2->addWidget(moveUpSelectedPb);
    moveDownSelectPb = new QPushButton;
    moveDownSelectPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_down.png")));
    connect(moveDownSelectPb, &QPushButton::clicked, this, &TrainNoDialog::moveDownTrain);
    vLayout2->addWidget(moveDownSelectPb);
    vLayout2->addStretch();

    addSelectedAllPb = new QPushButton;
    addSelectedAllPb->setIcon(QIcon(QStringLiteral(":/icon/images/double_arrow_right.png")));
    connect(addSelectedAllPb, &QPushButton::clicked, this, &TrainNoDialog::addSelectedTrainAll);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(addSelectedAllPb);

    addUnSelectedAllPb = new QPushButton;
    addUnSelectedAllPb->setIcon(QIcon(QStringLiteral(":/icon/images/double_arrow_left.png")));
    connect(addUnSelectedAllPb, &QPushButton::clicked, this, &TrainNoDialog::clearSelectedTrain);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(addUnSelectedAllPb);


    QVBoxLayout *vLayout3 = new QVBoxLayout;
    label = new QLabel(tr("已选中车次："));
    label->setFont(font);
    vLayout3->addWidget(label);
    vLayout3->addWidget(selected);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    QVBoxLayout *outvLayout = new QVBoxLayout;
    outvLayout->addLayout(hLayout);

    font.setPointSize(10);
    QVBoxLayout *vLayout4 = new QVBoxLayout;
    QButtonGroup *btnGroup = new QButtonGroup;
    QRadioButton *rb = new QRadioButton(tr("按选中车次的顺序提交"));
    rb->setFont(font);
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.strictTrainPrio = checked;
        QSettings setting;
        setting.setValue(_("train/strict_train"), checked);
    });
    QSettings setting;
    bool checked = setting.value(_("train/strict_train"), false).value<bool>();
    rb->setChecked(checked);
    rb->setFont(font);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    rb = new QRadioButton(tr("按列车发车时间顺序提交"));
    rb->setFont(font);
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.strictStartTimePrio = checked;
        QSettings setting;
        setting.setValue(_("train/strict_start_time"), checked);
    });
    checked = setting.value(_("train/strict_start_time"), false).value<bool>();
    rb->setChecked(checked);
    rb->setFont(font);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    rb = new QRadioButton(tr("行程时间短的车次优先提交"));
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.travelTimeShortPrio = checked;
        QSettings setting;
        setting.setValue(_("train/travel_time_short"), checked);
    });
    checked = setting.value(_("train/travel_time_short"), false).value<bool>();
    rb->setChecked(checked);
    rb->setFont(font);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    QRadioButton *suffientRb = new QRadioButton(tr("余票充足的车次优先提交"));
    QRadioButton *preferGRb = new QRadioButton(tr("G开头的优先提交"));
    QRadioButton *preferDRb = new QRadioButton(tr("D开头的优先提交"));
    QRadioButton *timeRangeRb = new QRadioButton(tr("以下时间段优先提交"));

    connect(suffientRb, &QRadioButton::toggled, this, [=](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.sufficientTicketPrio = checked;
        preferGRb->setEnabled(checked);
        preferDRb->setEnabled(checked);
        timeRangeRb->setEnabled(checked);
        QSettings setting;
        setting.setValue(_("train/sufficient_ticket"), checked);
    });
    checked = setting.value(_("train/sufficient_ticket"), true).value<bool>();
    suffientRb->setChecked(checked);
    suffientRb->setFont(font);
    preferGRb->setEnabled(checked);
    preferDRb->setEnabled(checked);
    timeRangeRb->setEnabled(checked);

    btnGroup->addButton(suffientRb);
    vLayout4->addWidget(suffientRb);
    //rb->setChecked(true);

    QVBoxLayout *vLayout5 = new QVBoxLayout;

    connect(preferGRb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.preferGPrio = checked;
        QSettings setting;
        setting.setValue(_("train/prefer_g"), checked);
    });
    checked = setting.value(_("train/prefer_g"), false).value<bool>();
    preferGRb->setChecked(checked);
    preferGRb->setFont(font);

    //btnGroup->addButton(rb);
    vLayout5->addWidget(preferGRb);

    connect(preferDRb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.preferDPrio = checked;
        QSettings setting;
        setting.setValue(_("train/prefer_d"), checked);
    });
    checked = setting.value(_("train/prefer_d"), false).value<bool>();
    preferDRb->setChecked(checked);
    preferDRb->setFont(font);
    //btnGroup->addButton(rb);
    vLayout5->addWidget(preferDRb);

    connect(timeRangeRb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.preferTimeRangePrio = checked;
        QSettings setting;
        setting.setValue(_("train/prefer_time_range"), checked);
    });
    checked = setting.value(_("train/prefer_time_range"), false).value<bool>();
    timeRangeRb->setChecked(checked);
    timeRangeRb->setFont(font);
    //btnGroup->addButton(rb);
    vLayout5->addWidget(timeRangeRb);

    vLayout4->setSpacing(1);
    vLayout5->setContentsMargins(2, 2, 2, 2);
    vLayout4->addStretch();
    outvLayout->addLayout(vLayout4);

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    QTimeEdit *timeEdit1 = new QTimeEdit;
    QTimeEdit *timeEdit2 = new QTimeEdit;
    connect(timeEdit1, &QDateTimeEdit::timeChanged, this, [=] (QTime time) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.timeRange.beginHour = time.hour();
        ud->grabSetting.trainPrio.timeRange.beginMinute = time.minute();
        QSettings setting;
        if (time >= timeEdit2->time()) {
            timeEdit2->setTime(QTime(time.hour() + 1, timeEdit2->time().minute(),
                                     timeEdit2->time().second()));
            setting.setValue(_("train/end_hour"), time.hour() + 1);
            setting.setValue(_("train/end_minute"), timeEdit2->time().minute());
        }
        setting.setValue(_("train/begin_hour"), time.hour());
        setting.setValue(_("train/begin_minute"), time.minute());
    });

    connect(timeEdit2, &QDateTimeEdit::timeChanged, this, [=] (QTime time) {
        UserData *ud = UserData::instance();
        ud->grabSetting.trainPrio.timeRange.endHour = time.hour();
        ud->grabSetting.trainPrio.timeRange.endMinute = time.minute();
        QSettings setting;

        if (time <= timeEdit1->time()) {
            timeEdit1->setTime(QTime(time.hour() - 1, timeEdit1->time().minute(),
                                     timeEdit1->time().second()));
            setting.setValue(_("train/begin_hour"), time.hour() - 1);
            setting.setValue(_("train/begin_minute"), timeEdit1->time().minute());
        }
        setting.setValue(_("train/end_hour"), time.hour());
        setting.setValue(_("train/end_minute"), time.minute());
    });
    timeEdit1->setDisplayFormat(QStringLiteral("hh:mm"));
    timeEdit1->setTime(QTime(8, 0));
    timeEdit1->setMaximumTime(QTime(22, 59, 0));
    timeEdit2->setDisplayFormat(QStringLiteral("hh:mm"));
    timeEdit2->setTime(QTime(12, 0));
    timeEdit2->setMaximumTime(QTime(23, 59, 0));
    timeEdit2->setMinimumTime(QTime(1, 0, 0));
    int hour = setting.value(_("train/begin_hour"), 8).value<int>();
    int minute = setting.value(_("train/begin_minute"), 0).value<int>();
    timeEdit1->setTime(QTime(hour, minute));
    hour = setting.value(_("train/end_hour"), 12).value<int>();
    minute = setting.value(_("train/end_minute"), 0).value<int>();
    timeEdit2->setTime(QTime(hour, minute));

    hLayout2->addWidget(timeEdit1);
    label = new QLabel(QStringLiteral("-"));
    hLayout2->addWidget(label);
    hLayout2->addWidget(timeEdit2);
    hLayout2->addStretch();
    hLayout2->setContentsMargins(30, 2, 2, 2);
    QGroupBox *groupBox = new QGroupBox;
    groupBox->setLayout(vLayout5);
    groupBox->setFlat(true);
    vLayout5->setSpacing(1);
    vLayout5->setContentsMargins(2, 2, 2, 2);
    vLayout5->addStretch();

    outvLayout->addWidget(groupBox);
    outvLayout->addLayout(hLayout2);
    outvLayout->addStretch();

    QPushButton *pb = new QPushButton(tr("确定"));
    connect(pb, &QPushButton::clicked, this, &TrainNoDialog::hide);
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    hLayout1->addWidget(pb);
    hLayout1->addStretch();

    outvLayout->addLayout(hLayout1);

    setLayout(outvLayout);
    setWindowTitle(tr("选择车次"));
    //trainNoDialog->resize(400, 400);
    //dialog->exec();
}

void TrainNoDialog::updateSelectedTips()
{
    QString tips = tr("已选%1/%2").arg(selected->count()).arg(trainSet.count());
    w->selectedTrainTipsLabel->setText(tips);
    addSelectedPb->setEnabled(unSelected->count() != 0);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    //QSettings setting;
    //setting.setValue(_("train/selected_train"), getSelectedTrainList().toList());
}

bool TrainNoDialog::hasTrain(const QString &trainInfo)
{
    return trainSet.contains(trainInfo);
}

void TrainNoDialog::addTrain(const QString &trainInfo, const QString &display)
{
    if (!trainInfo.isEmpty()) {
        if (!hasTrain(trainInfo)) {
            QListWidgetItem *item = new QListWidgetItem(unSelected);
            item->setData(Qt::DisplayRole, display);
            unSelected->setCurrentItem(item);
            addSelectedPb->setEnabled(true);
            trainSet.insert(trainInfo);
        }
    }
}

void TrainNoDialog::addTrainFinish()
{
    if (UserData::instance()->runStatus == EIDLE) {
        updateSelectedTips();
        unSelected->setCurrentRow(0);
    }
}

void TrainNoDialog::addSelectedTrain(const QString &trainInfo)
{
    if (!trainInfo.isEmpty()) {
        QList<QListWidgetItem *> itemList = selected->findItems(trainInfo + ' ', Qt::MatchStartsWith);
        if (itemList.isEmpty()) {
            itemList = unSelected->findItems(trainInfo + ' ', Qt::MatchStartsWith);
            if (itemList.isEmpty()) {
                return;
            }
            QListWidgetItem *item = unSelected->takeItem(unSelected->row(itemList[0]));
            selected->addItem(item);
            //QStringList textList = item->text().split(' ');
            //selectedTrainSet.insert(_("%1 %2 %3").arg(textList[0], textList[1], textList[2]));
            selectedTrainSet.insert(trainInfo);
        }
        updateSelectedTips();
    }
}

void TrainNoDialog::removeSelectedTrain(const QString &trainInfo)
{
    if (!trainInfo.isEmpty()) {
        QList<QListWidgetItem *> itemList = unSelected->findItems(trainInfo + ' ', Qt::MatchStartsWith);
        if (itemList.isEmpty()) {
            itemList = selected->findItems(trainInfo + ' ', Qt::MatchStartsWith);
            if (itemList.isEmpty()) {
                return;
            }
            QListWidgetItem *item = selected->takeItem(selected->row(itemList[0]));
            unSelected->addItem(item);
            //QStringList textList = item->text().split(' ');
            //selectedTrainSet.remove(_("%1 %2 %3").arg(textList[0], textList[1], textList[2]));
            selectedTrainSet.insert(trainInfo);
        }
        updateSelectedTips();
    }
}

void TrainNoDialog::addSelectedTrainAll()
{
    if (unSelected->count()) {
        unSelected->setCurrentRow(unSelected->count() - 1);
        while (unSelected->count()) {
            QListWidgetItem *item = unSelected->takeItem(unSelected->currentRow());
            selected->addItem(item);
            QStringList textList = item->text().split(' ');
            selectedTrainSet.insert(_("%1 %2 %3").arg(textList[0], textList[1], textList[2]));
        }
        selected->setCurrentRow(0);
        updateSelectedTips();
    }
}

void TrainNoDialog::clearSelectedTrain()
{
    if (selected->count()) {
        selected->setCurrentRow(selected->count() - 1);
        while (selected->count()) {
            QListWidgetItem *item = selected->takeItem(selected->currentRow());
            unSelected->addItem(item);
            QStringList textList = item->text().split(' ');
            selectedTrainSet.remove(_("%1 %2 %3").arg(textList[0], textList[1], textList[2]));
        }
        unSelected->setCurrentRow(0);
        updateSelectedTips();
    }
}

void TrainNoDialog::clearUnSelectedTrain()
{
    if (unSelected->count()) {
        unSelected->setCurrentRow(unSelected->count() - 1);
        while (unSelected->count()) {
            QListWidgetItem *item = unSelected->takeItem(unSelected->currentRow());
            delete item;
        }
        updateSelectedTips();
        trainSet.clear();
    }
}

const QList<QString> &TrainNoDialog::getSelectedTrainList() const
{
    static QList<QString> trainNoList;
    trainNoList.clear();
    trainNoList.resize(selected->count());
    int idx = 0;
    for (int i = 0; i < selected->count(); i++) {
        const QString &s = selected->item(i)->text();
        int j, k;
        for (j = 0, k = 0; j < s.size(); j++) {
            if (s[j] == ' ')
                k++;
            if (k == 3)
                break;
        }
        if (j < s.size()) {
            trainNoList[idx++] = s.first(j);
        }
    }
    return trainNoList;
}

const QSet<QString> &TrainNoDialog::getSelectedTrainSet() const
{
    return selectedTrainSet;
}

const QSet<QString> &TrainNoDialog::getAllTrainSet() const
{
    return trainSet;
}

void TrainNoDialog::setSelectedTrainNo()
{
    QListWidgetItem *item = unSelected->currentItem();
    if (item) {
        QList<QListWidgetItem *> list = selected->findItems(item->text(), Qt::MatchExactly);
        if (list.isEmpty()) {
            //QListWidgetItem *item2 = new QListWidgetItem(selected);
            //item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
            //selected->setCurrentItem(unSelected->takeItem(unSelected->currentRow()));
            //delete unSelected->takeItem(unSelected->currentRow());
            QListWidgetItem *item = unSelected->takeItem(unSelected->currentRow());
            selected->addItem(item);
            selected->setCurrentItem(item);
            QStringList textList = item->text().split(' ');
            selectedTrainSet.insert(_("%1 %2 %3").arg(textList[0], textList[1], textList[2]));
        }
    }
    updateSelectedTips();
}

void TrainNoDialog::setUnselectedTrainNo()
{
    QListWidgetItem *item = selected->currentItem();

    if (item) {
        //QListWidgetItem *item2 = new QListWidgetItem(unSelected);
        //item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
        //unSelected->setCurrentItem(selected->takeItem(selected->currentRow()));
        //delete selected->takeItem(selected->currentRow());
        QListWidgetItem *item = selected->takeItem(selected->currentRow());
        unSelected->addItem(item);
        unSelected->setCurrentItem(item);
        QStringList textList = item->text().split(' ');
        selectedTrainSet.remove(_("%1 %2 %3").arg(textList[0], textList[1], textList[2]));
    }

    updateSelectedTips();
}

void TrainNoDialog::moveUpTrain()
{
    QListWidgetItem *item = selected->currentItem();
    if (item) {
        int row = selected->row(item);
        if (row > 0) {
            selected->takeItem(row);
            selected->insertItem(row - 1, item);
            selected->setCurrentItem(item);
        }
    }
}

void TrainNoDialog::moveDownTrain()
{
    QListWidgetItem *item = selected->currentItem();
    if (item) {
        int row = selected->row(item);
        if (row < selected->count() - 1) {
            selected->takeItem(row);
            selected->insertItem(row + 1, item);
            selected->setCurrentItem(item);
        }
    }
}

void TrainNoDialog::enterGrabTicketMode()
{
    addSelectedPb->setEnabled(false);
    addUnSelectedPb->setEnabled(false);
    addSelectedAllPb->setEnabled(false);
    addUnSelectedAllPb->setEnabled(false);
    moveUpSelectedPb->setEnabled(false);
    moveDownSelectPb->setEnabled(false);
}

void TrainNoDialog::exitGrabTicketMode()
{
    addSelectedPb->setEnabled(unSelected->count() != 0);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    moveUpSelectedPb->setEnabled(true);
    moveDownSelectPb->setEnabled(true);
}

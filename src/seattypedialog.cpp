#include "seattypedialog.h"
#include "userdata.h"
#include "12306.h"
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTimer>
#include "mainwindow.h"
#include <QPainter>
#include <QSettings>

#define _ QStringLiteral

extern MainWindow *w;

SeatTypeDialog::SeatTypeDialog(QWidget *parent) :
    QDialog(parent)
{
}

SeatTypeDialog::~SeatTypeDialog()
{
}

void SeatTypeDialog::setUp()
{
    unSelected = new QListWidget;
    unSelected->setMinimumWidth(100);
    selected = new QListWidget;
    selected->setMinimumWidth(100);
    QLabel *label = new QLabel(tr("未选中席别："));

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

    QList<QString> seatTypeList = {
        QStringLiteral("一等座"),
        QStringLiteral("二等座"),
        QStringLiteral("特等座"),
        QStringLiteral("高级软卧"),
        QStringLiteral("软卧"),
        QStringLiteral("动卧"),
        QStringLiteral("硬卧"),
        QStringLiteral("硬座"),
        QStringLiteral("无座")
    };

    /*QVector<QString> seatTypeStrVec;
    seatTypeStrVec.push_back(QStringLiteral("特等座"));
    seatTypeStrVec.push_back(QStringLiteral("一等座"));
    seatTypeStrVec.push_back(QStringLiteral("二等座"));
    seatTypeStrVec.push_back(QStringLiteral("高级软卧"));
    seatTypeStrVec.push_back(QStringLiteral("软卧"));
    seatTypeStrVec.push_back(QStringLiteral("动卧"));
    seatTypeStrVec.push_back(QStringLiteral("硬卧"));
    seatTypeStrVec.push_back(QStringLiteral("硬座"));
    seatTypeStrVec.push_back(QStringLiteral("无座"));*/

    QSettings setting;
    QList<QVariant> selSeatTypeList = setting.value(_("seat_type/selected_seat_type")).toList();
    for (auto &seatType : selSeatTypeList) {
        QListWidgetItem *item = new QListWidgetItem(selected);
        item->setData(Qt::DisplayRole, seatType);
    }

    for (auto &seatType : seatTypeList) {
        int i;
        for (i = 0; i < selected->count(); i++) {
            if (selected->item(i)->text() == seatType) {
                break;
            }
        }
        if (i == selected->count()) {
            QListWidgetItem *item = new QListWidgetItem(unSelected);
            item->setData(Qt::DisplayRole, seatType);
        }
    }

#if 0
    for (it = grabSetting.seatTypeCode.cbegin();
         it != grabSetting.seatTypeCode.cend(); ++it) {
        QString sd = seatTypeCodeToName(*it);
        if (!sd.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem(selected);
            item->setData(Qt::DisplayRole, sd);
        }
    }

    QMap<QString, QChar>::const_iterator it2;
    for (it2 = seatTypeMap.cbegin(); it2 != seatTypeMap.cend(); ++it2) {
        if (!grabSetting.seatTypeCode.contains(it2.value())) {
            QListWidgetItem *item = new QListWidgetItem(unSelected);
            item->setData(Qt::DisplayRole, it2.key());
        }
    }
#endif
    unSelected->setCurrentRow(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(label);
    vLayout->addWidget(unSelected);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addStretch();
    addSelectedPb = new QPushButton;
    addSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_right.png")));
    connect(addSelectedPb, &QPushButton::clicked, this, &SeatTypeDialog::setSelectedSeatType);
    addSelectedPb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(addSelectedPb);

    addUnSelectedPb = new QPushButton;
    addUnSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_left.png")));
    connect(addUnSelectedPb, &QPushButton::clicked, this, &SeatTypeDialog::setUnselectedSeatType);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(addUnSelectedPb);
    vLayout2->addStretch();
    moveUpSelectedPb = new QPushButton;
    moveUpSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_up.png")));
    connect(moveUpSelectedPb, &QPushButton::clicked, this, &SeatTypeDialog::moveUpSeatType);
    vLayout2->addWidget(moveUpSelectedPb);
    moveDownSelectPb = new QPushButton;
    moveDownSelectPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_down.png")));
    connect(moveDownSelectPb, &QPushButton::clicked, this, &SeatTypeDialog::moveDownSeatType);
    vLayout2->addWidget(moveDownSelectPb);
    vLayout2->addStretch();
    addSelectedAllPb = new QPushButton;
    addSelectedAllPb->setIcon(QIcon(QStringLiteral(":/icon/images/double_arrow_right.png")));
    connect(addSelectedAllPb, &QPushButton::clicked, this, &SeatTypeDialog::clearSelectedSeatType);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(addSelectedAllPb);

    addUnSelectedAllPb = new QPushButton;
    addUnSelectedAllPb->setIcon(QIcon(QStringLiteral(":/icon/images/double_arrow_left.png")));
    connect(addUnSelectedAllPb, &QPushButton::clicked, this, &SeatTypeDialog::clearUnSelectedSeatType);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(addUnSelectedAllPb);

    QVBoxLayout *vLayout3 = new QVBoxLayout;
    label = new QLabel(tr("已选中席别："));
    vLayout3->addWidget(label);
    vLayout3->addWidget(selected);
#if 0
    QImage *image = new QImage(18, 18, QImage::Format_RGB32);

    QPainter painter(image); // 创建一个绘制器并将它与 Image 关联起来

    painter.fillRect(image->rect(), QBrush(QColor(255,127,36))); // 填充矩形背景

    QPen pen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    //painter.drawLine(50, 250, 200, 100);  // 绘制直线
    //painter.drawRect(0, 0, 15, 15);  // 绘制矩形
    //painter.drawEllipse(250, 50, 100, 150);  // 绘制椭圆
    QFont font;
    font.setPointSize(12);
    //font.setBold(true);
    painter.setFont(font);

    QString text = tr("动");
    QRect rect = painter.boundingRect(1, 1, 16, 16, Qt::AlignCenter, text);

    painter.drawText(rect, text);


    painter.end();    // 和上面一样，必须要调用 end() 结束绘画
    image->save("1.ico", "ICO");
    delete image;
#endif
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    QVBoxLayout *outvLayout = new QVBoxLayout;
    outvLayout->addLayout(hLayout);

    QVBoxLayout *vLayout4 = new QVBoxLayout;
    QButtonGroup *btnGroup = new QButtonGroup;
    QRadioButton *rb = new QRadioButton(tr("按选中席别的顺序提交"));
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.seatTypePrio.strictSeatTypePrio = checked;
        QSettings setting;
        setting.setValue(_("seat_type/strict_seat_type"), checked);
    });
    bool checked = setting.value(_("seat_type/strict_seat_type"), false).value<bool>();
    rb->setChecked(checked);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    rb = new QRadioButton(tr("余票充足的席别优先提交"));
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.seatTypePrio.sufficientSeatTypePrio = checked;
        QSettings setting;
        setting.setValue(_("seat_type/sufficient_seat_type"), checked);
    });
    checked = setting.value(_("seat_type/sufficient_seat_type"), false).value<bool>();
    rb->setChecked(checked);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    rb = new QRadioButton(tr("余票充足时价格低的席别优先提交"));
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.seatTypePrio.priceLowSeatTypePrioWhensufficient = checked;
        QSettings setting;
        setting.setValue(_("seat_type/price_low_sufficient"), checked);
    });
    checked = setting.value(_("seat_type/price_low_sufficient"), true).value<bool>();
    rb->setChecked(checked);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    //rb->setChecked(true);
    rb = new QRadioButton(tr("价格低的席别优先提交"));
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.seatTypePrio.priceLowSeatTypePrio = checked;
        QSettings setting;
        setting.setValue(_("seat_type/price_low_seat_type"), checked);
    });
    checked = setting.value(_("seat_type/price_low_seat_type"), false).value<bool>();
    rb->setChecked(checked);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    rb = new QRadioButton(tr("价格高的席别优先提交"));
    connect(rb, &QRadioButton::toggled, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.seatTypePrio.priceHighSeatTypePrio = checked;
        QSettings setting;
        setting.setValue(_("seat_type/price_high_seat_type"), checked);
    });
    checked = setting.value(_("seat_type/price_high_seat_type"), false).value<bool>();
    rb->setChecked(checked);
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);

    outvLayout->addLayout(vLayout4);

    QPushButton *pb = new QPushButton(tr("确定"));
    connect(pb, &QPushButton::clicked, this, &SeatTypeDialog::hide);
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    hLayout1->addWidget(pb);
    hLayout1->addStretch();

    outvLayout->addLayout(hLayout1);

    setLayout(outvLayout);
    setWindowTitle(tr("选择席别"));
    //seatTypeDialog->resize(400, 400);
    //dialog->exec();
    QTimer *timer = new QTimer;
    connect(timer, &QTimer::timeout, this, [=] {
        updateSelectedTips();
        timer->stop();
        delete timer;
    });
    timer->setInterval(1000);
    timer->start();
}

void SeatTypeDialog::updateSelectedTips()
{
    QString tips = tr("已选%1/%2").arg(selected->count()).arg(selected->count() + unSelected->count());
    w->selectedSeatTypeTipsLabel->setText(tips);
    addSelectedPb->setEnabled(unSelected->count() != 0);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    QSettings setting;
    setting.setValue(_("seat_type/selected_seat_type"), getSelectedSeatType().toList());
}

const QList<QString> &SeatTypeDialog::getSelectedSeatType() const
{
    static QList<QString> seatTypeList;
    seatTypeList.clear();
    seatTypeList.resize(selected->count());
    for (int i = 0; i < selected->count(); i++) {
        seatTypeList[i] = selected->item(i)->text();
    }

    return seatTypeList;
}

void SeatTypeDialog::setSelectedSeatType()
{
    //UserData *ud = UserData::instance();

    QListWidgetItem *item = unSelected->currentItem();
    if (item) {
        QList<QListWidgetItem *> list = selected->findItems(item->text(), Qt::MatchExactly);
        if (list.isEmpty()) {
            //QListWidgetItem *item2 = new QListWidgetItem(selected);
            //item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
            // selected->setCurrentItem(unSelected->takeItem(unSelected->currentRow()));
            //delete unSelected->takeItem(unSelected->currentRow());
            QListWidgetItem *item = unSelected->takeItem(unSelected->currentRow());
            selected->addItem(item);
            selected->setCurrentItem(item);
        }
    }

    updateSelectedTips();
}

void SeatTypeDialog::setUnselectedSeatType()
{
    //UserData *ud = UserData::instance();

    QListWidgetItem *item = selected->currentItem();

    if (item) {
        QList<QListWidgetItem *> list = unSelected->findItems(item->text(), Qt::MatchExactly);
        if (list.isEmpty()) {
            //QListWidgetItem *item2 = new QListWidgetItem(unSelected);
            //item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
            QListWidgetItem *item = selected->takeItem(selected->currentRow());
            unSelected->addItem(item);
            unSelected->setCurrentItem(item);
            //delete selected->takeItem(selected->currentRow());
        }
    }

    updateSelectedTips();
}

void SeatTypeDialog::clearSelectedSeatType()
{
    if (unSelected->count()) {
        unSelected->setCurrentRow(unSelected->count() - 1);
        while (unSelected->count()) {
            QListWidgetItem *item = unSelected->takeItem(unSelected->currentRow());
            selected->addItem(item);
        }
        selected->setCurrentRow(0);
        updateSelectedTips();
    }
}

void SeatTypeDialog::clearUnSelectedSeatType()
{
    if (selected->count()) {
        selected->setCurrentRow(selected->count() - 1);
        while (selected->count()) {
            QListWidgetItem *item = selected->takeItem(selected->currentRow());
            unSelected->addItem(item);
        }
        unSelected->setCurrentRow(0);
        updateSelectedTips();
    }
}

void SeatTypeDialog::moveUpSeatType()
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

void SeatTypeDialog::moveDownSeatType()
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

void SeatTypeDialog::enterGrabTicketMode()
{
    addSelectedPb->setEnabled(false);
    addUnSelectedPb->setEnabled(false);
    addSelectedAllPb->setEnabled(false);
    addUnSelectedAllPb->setEnabled(false);
    moveUpSelectedPb->setEnabled(false);
    moveDownSelectPb->setEnabled(false);
}

void SeatTypeDialog::exitGrabTicketMode()
{
    addSelectedPb->setEnabled(unSelected->count() != 0);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    moveUpSelectedPb->setEnabled(true);
    moveDownSelectPb->setEnabled(true);
}

QString SeatTypeDialog::seatTypeCodeToName(QChar code)
{
    switch (code.toLatin1()) {
    case SEATTEDENG:
        return QStringLiteral("特等座");
    case SEATYIDENG:
        return QStringLiteral("一等座");
    case SEATERDENG:
        return QStringLiteral("二等座");
    case SEATGAOJIDONGWUO:
        return QStringLiteral("高级软卧");
    case SEATRUANWUO:
        return QStringLiteral("软卧");
    case SEATDONGWUO:
        return QStringLiteral("动卧");
    case SEATYINGWUO:
        return QStringLiteral("硬卧");
    case SEATYINGZUO:
        return QStringLiteral("硬座");
    default:
        return QStringLiteral("");
    }
    return QStringLiteral("");
}

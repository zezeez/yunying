#include "passengerdialog.h"
#include "userdata.h"
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QSettings>
#include "mainwindow.h"
#include "nethelper.h"

extern MainWindow *w;

PassengerDialog::PassengerDialog(QWidget *parent) :
    QDialog(parent)
{
}

PassengerDialog::~PassengerDialog()
{
}

void PassengerDialog::setUp()
{
    unSelected = new QListWidget;
    unSelected->setMinimumWidth(100);
    selected = new QListWidget;
    selected->setMinimumWidth(100);
    QLabel *label = new QLabel(tr("未选中的乘车人："));

    //unSelected->setViewMode(QListWidget::IconMode );
    //unSelected->setResizeMode(QListWidget::Adjust);
    //unSelected->setMovement(QListWidget::Static);
    /*UserData *ud = UserData::instance();
    struct UserLoginInfo &info = ud->getUserLoginInfo();
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
    }*/

    unSelected->setCurrentRow(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(label);
    vLayout->addWidget(unSelected);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addStretch();
    addSelectedPb = new QPushButton;
    addSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_right.png")));
    connect(addSelectedPb, &QPushButton::clicked, this, &PassengerDialog::setSelectedPassenger);
    addSelectedPb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(addSelectedPb);

    addUnSelectedPb = new QPushButton;
    addUnSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_left.png")));
    connect(addUnSelectedPb, &QPushButton::clicked, this, &PassengerDialog::setUnselectedPassenger);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(addUnSelectedPb);
    vLayout2->addStretch();
    moveUpSelectedPb = new QPushButton;
    moveUpSelectedPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_up.png")));
    connect(moveUpSelectedPb, &QPushButton::clicked, this, &PassengerDialog::moveUpPassenger);
    vLayout2->addWidget(moveUpSelectedPb);
    moveDownSelectPb = new QPushButton;
    moveDownSelectPb->setIcon(QIcon(QStringLiteral(":/icon/images/arrow_down.png")));
    connect(moveDownSelectPb, &QPushButton::clicked, this, &PassengerDialog::moveDownPassenger);
    vLayout2->addWidget(moveDownSelectPb);
    vLayout2->addStretch();
    addSelectedAllPb = new QPushButton;
    addSelectedAllPb->setIcon(QIcon(QStringLiteral(":/icon/images/double_arrow_right.png")));
    connect(addSelectedAllPb, &QPushButton::clicked, this, &PassengerDialog::clearUnSelectedPassenger);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    vLayout2->addWidget(addSelectedAllPb);

    addUnSelectedAllPb = new QPushButton;
    addUnSelectedAllPb->setIcon(QIcon(QStringLiteral(":/icon/images/double_arrow_left.png")));
    connect(addUnSelectedAllPb, &QPushButton::clicked, this, &PassengerDialog::clearSelectedPassenger);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    vLayout2->addWidget(addUnSelectedAllPb);

    QVBoxLayout *vLayout3 = new QVBoxLayout;
    label = new QLabel(tr("已选中的乘车人："));
    vLayout3->addWidget(label);
    vLayout3->addWidget(selected);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);
    hLayout->addLayout(vLayout3);

    QVBoxLayout *outvLayout = new QVBoxLayout;
    outvLayout->addLayout(hLayout);

    QVBoxLayout *vLayout4 = new QVBoxLayout;
    QButtonGroup *btnGroup = new QButtonGroup;
    QRadioButton *rb;
    label = new QLabel(QStringLiteral("余票不足时："));
    vLayout4->addWidget(label);
    rb = new QRadioButton(QStringLiteral("部分提交(按选中乘车人的顺序)"));
    connect(rb, &QRadioButton::toggled, this, [] (bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.ticketSetting.partialSubmit = checked;
        QSettings setting;
        setting.setValue(_("ticket/partial_submit"), checked);
    });
    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
    rb->setChecked(true);
    UserData *ud = UserData::instance();
    ud->grabSetting.ticketSetting.partialSubmit = true;
    rb = new QRadioButton(QStringLiteral("不提交"));

    btnGroup->addButton(rb);
    vLayout4->addWidget(rb);
#if 0
    QCheckBox *chkBox = new QCheckBox(QStringLiteral("余票无座时不提交"));
    connect(chkBox, &QCheckBox::clicked, this, [&](bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.ticketSetting.noSubmitNoSeat = checked;
    });
    vLayout4->addWidget(chkBox);
#endif

    outvLayout->addLayout(vLayout4);

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    refreshPassengerPb = new QPushButton(tr("刷新"));
    connect(refreshPassengerPb, &QPushButton::clicked, this, &PassengerDialog::refreshPassengerInfo);
    refreshPassengerPb->setToolTip(tr("若左侧乘车人未显示请刷新一遍"));
    hLayout1->addWidget(refreshPassengerPb);
    QPushButton *pb = new QPushButton(tr("确定"));
    connect(pb, &QPushButton::clicked, this, &PassengerDialog::hide);
    hLayout1->addWidget(pb);
    hLayout1->addStretch();

    outvLayout->addLayout(hLayout1);

    setLayout(outvLayout);
    setWindowTitle(tr("选择乘车人"));
    //passengerDialog->resize(350, 200);
    //dialog->exec();
}

void PassengerDialog::refreshPassengerInfo()
{
    NetHelper::instance()->getPassengerInfo();
}

void PassengerDialog::updateSelectedTips()
{
    QString tips = tr("已选%1/%2").arg(selected->count()).arg(selected->count() + unSelected->count());
    w->selectedPassengerTipsLabel->setText(tips);
    addSelectedPb->setEnabled(unSelected->count() != 0);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
}

const QList<QString> &PassengerDialog::getSelectedPassenger() const
{
    static QList<QString> passengerList;
    passengerList.clear();
    passengerList.resize(selected->count());
    for (int i = 0; i < selected->count(); i++) {
        passengerList[i] = selected->item(i)->text();
    }
    return passengerList;
}

void PassengerDialog::addSelectedPassenger(const QString &passengerName)
{
    QList<QListWidgetItem *> itemList = unSelected->findItems(passengerName, Qt::MatchExactly);
    for (auto &i : itemList) {
        unSelected->removeItemWidget(i);
    }
    selected->addItem(passengerName);
    selected->setCurrentRow(0);
    updateSelectedTips();
}

void PassengerDialog::addUnSelectedPassenger(const QString &passengerName)
{
    QList<QListWidgetItem *> itemList = selected->findItems(passengerName, Qt::MatchExactly);
    for (auto &i : itemList) {
        selected->removeItemWidget(i);
    }
    unSelected->addItem(passengerName);
    unSelected->setCurrentRow(0);
    updateSelectedTips();
}

void PassengerDialog::setSelectedPassenger()
{
    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QVector<struct PassengerInfo>::const_iterator it;
    QListWidgetItem *item = unSelected->currentItem();
    if (item) {
        QList<QListWidgetItem *> list = selected->findItems(item->text(), Qt::MatchExactly);
        if (list.isEmpty()) {
            for (it = ud->passenger.cbegin(); it != ud->passenger.cend(); ++it) {
                if (!it->passName.compare(item->text())) {
                    if (!grabSetting.selectedPassenger.contains(*it)) {
                        grabSetting.selectedPassenger.push_back(*it);
                    }
                    break;
                }
            }
            if (it == ud->passenger.cend() && !grabSetting.selectedDjPassenger.isEmpty()) {
                for (it = ud->djPassenger.cbegin(); it != ud->djPassenger.cend(); ++it) {
                    if (!it->passName.compare(item->text())) {
                        if (!grabSetting.selectedDjPassenger.contains(*it)) {
                            grabSetting.selectedDjPassenger.push_back(*it);
                        }
                        break;
                    }
                }
            }

            //QListWidgetItem *item2 = new QListWidgetItem(selected);
            //item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
            //selected->setCurrentItem(unSelected->takeItem(unSelected->currentRow()));
            //delete unSelected->takeItem(unSelected->currentRow());
            QListWidgetItem *item = unSelected->takeItem(unSelected->currentRow());
            selected->addItem(item);
            selected->setCurrentItem(item);
        }
    }

    updateSelectedTips();
}

void PassengerDialog::setUnselectedPassenger()
{
    UserData *ud = UserData::instance();
    struct GrabTicketSetting &grabSetting = ud->getGrabTicketSetting();

    QVector<struct PassengerInfo>::const_iterator it;
    QListWidgetItem *item = selected->currentItem();

    if (item) {
        for (it = ud->passenger.cbegin(); it != ud->passenger.cend(); ++it) {
            if (!it->passName.compare(item->text())) {
                if (grabSetting.selectedPassenger.contains(*it)) {
                    grabSetting.selectedPassenger.removeOne(*it);
                }
                break;
            }
        }
        for (it = ud->djPassenger.cbegin(); it != ud->djPassenger.cend(); ++it) {
            if (!it->passName.compare(item->text())) {
                if (grabSetting.selectedDjPassenger.contains(*it)) {
                    grabSetting.selectedDjPassenger.removeOne(*it);
                }
                break;
            }
        }
        //QListWidgetItem *item2 = new QListWidgetItem(unSelected);
        //item2->setData(Qt::DisplayRole, QObject::tr("%1").arg(item->text()));
        //unSelected->setCurrentItem(selected->takeItem(selected->currentRow()));
        //delete selected->takeItem(selected->currentRow());
        QListWidgetItem *item = selected->takeItem(selected->currentRow());
        unSelected->addItem(item);
        unSelected->setCurrentItem(item);
    }

    updateSelectedTips();
}

void PassengerDialog::clearUnSelectedPassenger()
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

void PassengerDialog::clearSelectedPassenger()
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

void PassengerDialog::clearPassenger()
{
    if (selected->count()) {
        selected->setCurrentRow(selected->count() - 1);
        while (selected->count()) {
            QListWidgetItem *item = selected->takeItem(selected->currentRow());
            delete item;
        }
        updateSelectedTips();
    }
    if (unSelected->count()) {
        unSelected->setCurrentRow(unSelected->count() - 1);
        while (unSelected->count()) {
            QListWidgetItem *item = unSelected->takeItem(unSelected->currentRow());
            delete item;
        }
        updateSelectedTips();
    }
}

void PassengerDialog::moveUpPassenger()
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

void PassengerDialog::moveDownPassenger()
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

void PassengerDialog::enterGrabTicketMode()
{
    addSelectedPb->setEnabled(false);
    addUnSelectedPb->setEnabled(false);
    addSelectedAllPb->setEnabled(false);
    addUnSelectedAllPb->setEnabled(false);
    moveUpSelectedPb->setEnabled(false);
    moveDownSelectPb->setEnabled(false);
    refreshPassengerPb->setEnabled(false);
}

void PassengerDialog::exitGrabTicketMode()
{
    addSelectedPb->setEnabled(unSelected->count() != 0);
    addUnSelectedPb->setEnabled(selected->count() != 0);
    addSelectedAllPb->setEnabled(unSelected->count() != 0);
    addUnSelectedAllPb->setEnabled(selected->count() != 0);
    moveUpSelectedPb->setEnabled(true);
    moveDownSelectPb->setEnabled(true);
    refreshPassengerPb->setEnabled(true);
}

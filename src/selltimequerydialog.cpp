#include "selltimequerydialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include "nethelper.h"
#include "userdata.h"

SellTimeQueryDialog::SellTimeQueryDialog(QWidget *parent) :
    QDialog(parent)
{

}

SellTimeQueryDialog::~SellTimeQueryDialog()
{

}

void SellTimeQueryDialog::setup()
{
    QVBoxLayout *vlayout = new QVBoxLayout;
    QHBoxLayout *hlayout = new QHBoxLayout;
    QLabel *la = new QLabel(tr("起售车站: "));
    queryStaLe = new QLineEdit;
    QPushButton *pb = new QPushButton(tr("查询"));
    msgLabel = new QLabel;
    msgLabel->setStyleSheet(_("color: red"));
    dispLabel = new QLabel;
    connect(pb, &QPushButton::clicked, this, &SellTimeQueryDialog::queryScName);
    hlayout->addWidget(la);
    hlayout->addWidget(queryStaLe);
    hlayout->addWidget(pb);
    hlayout->addStretch();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(msgLabel);
    vlayout->addWidget(dispLabel);
    vlayout->addStretch();
    setLayout(vlayout);
    resize(450, 300);
}

QString SellTimeQueryDialog::getInputStaCode()
{
    QString staCode;
    QString staName = queryStaLe->text().trimmed();
    if (staName.isEmpty()) {
        msgLabel->setText(_("请先输入起售车站"));
        return staCode;
    }
    UserData *ud = UserData::instance();
    staCode = ud->getStaCode()->value(staName);
    if (staCode.isEmpty()) {
        msgLabel->setText(_("该站点不存在，请更换站点后再试"));
        return staCode;
    }
    msgLabel->clear();
    return staCode;
}

void SellTimeQueryDialog::queryScName()
{
    cityStaName.clear();
    dispLabel->clear();
    QString staCode = getInputStaCode();
    if (staCode.isEmpty()) {
        return;
    }
    NetHelper::instance()->queryScName(staCode);
}

void SellTimeQueryDialog::queryScNameReply(QVariantMap &varMap)
{
    QStringList data = varMap[_("data")].toStringList();
    if (data.isEmpty()) {
        msgLabel->setText(_("暂无查询结果"));
        return;
    }
    for (int i = 0; i < data.size(); i++) {
        QStringList part = data[i].split(',');
        if (part.size() < 2) {
            continue;
        }
        cityStaName.insert(part[0], part[1]);
    }
    queryAllCacheSaleTime();
}

void SellTimeQueryDialog::queryAllCacheSaleTime()
{
    NetHelper::instance()->queryAllCacheSaleTime();
}

void SellTimeQueryDialog::queryAllCacheSaleTimeReply(QVariantMap &varMap)
{
    QString dispMsg;
    int count = 0;
    bool status = varMap[_("status")].toBool();
    if (!status) {
        msgLabel->setText(_("查询失败，返回结果错误"));
        return;
    }
    QVariantList data = varMap[_("data")].toList();
    if (data.isEmpty()) {
        msgLabel->setText(_("暂无查询结果"));
        return;
    }
    QMap<QString, QString>::const_iterator it;
    for (it = cityStaName.cbegin(); it != cityStaName.cend(); it++) {
        QString staCode = it.key();
        for (int j = 0; j < data.size(); j++) {
            QVariantMap itemMap = data[j].toMap();
            if (itemMap[_("station_telecode")] == staCode) {
                QString saleTime = itemMap[_("sale_time")].toString();
                if (saleTime.size() >= 4) {
                    dispMsg += it.value() + _(": ") + saleTime.sliced(0, 2) + _(":") + saleTime.sliced(2, 2) + _("    ");
                    count++;
                    if (count > 3) {
                        count = 0;
                        dispMsg += _("\n");
                    }
                }
                break;
            }
        }
    }
    dispLabel->setText(dispMsg);
    queryLcTime();
}

void SellTimeQueryDialog::queryLcTime()
{
    QString staCode = getInputStaCode();
    if (staCode.isEmpty()) {
        return;
    }

    QString date = UserData::instance()->userConfig.tourDate;
    if (!date.isEmpty()) {
        date.remove('-');
    } else {
        date = QDate::currentDate().toString(_("yyyyMMdd"));
    }
    NetHelper::instance()->queryLcTime(staCode, date);
}

void SellTimeQueryDialog::queryLcTimeReply(QVariantMap &varMap)
{
    QString dispMsg;
    int count = 0;
    QVariantList data = varMap[_("data")].toList();
    if (data.isEmpty()) {
        return;
    }
    qDebug() << cityStaName;
    qDebug() << data;
    dispMsg += _("\n\n同城换乘时间参考: \n");
    for (int i = 0; i < data.size(); i++) {
        QVariantMap itemMap = data[i].toMap();
        QString staFromCode = itemMap[_("from_station_telecode")].toString();
        QString staToCode = itemMap[_("to_station_telecode")].toString();
        QString staFromName = cityStaName.value(staFromCode);
        QString staToName = cityStaName.value(staToCode);
        int relayTime = itemMap[_("relay_time")].toInt();
        if (!staFromName.isEmpty() && !staToName.isEmpty()) {
            dispMsg += _("%1->%2(%3): ").arg(staFromName, staToName, staFromCode != staToCode ? _("同城") : _("站内"));
            int hours, minutes;
            hours = relayTime > 60 ? relayTime / 60 : 0;
            minutes = relayTime % 60;
            dispMsg += hours > 0 ? _("%1小时%2分钟").arg(hours).arg(minutes) + _("    "): _("%1分钟").arg(minutes) + _("    ");
            count++;
            if (count > 1) {
                count = 0;
                dispMsg += _("\n");
            }
        }
    }
    dispLabel->setText(dispLabel->text() + dispMsg);
}

void SellTimeQueryDialog::setQueryText(const QString &text)
{
    queryStaLe->setText(text);
}

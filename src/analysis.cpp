#include "include/analysis.h"
#include "userdata.h"
#include "12306.h"
#include "mainwindow.h"
#include "nethelper.h"

#ifdef __GNUC__
#define TICKETIDX(idx) ({ Q_ASSERT(idx >= EGGNUM && idx < EYPEX); ((idx) - EGGNUM); })
#else
#define TICKETIDX(idx) ((idx) - EGGNUM)
#endif
#define TICKETSIZE (EYPEX - EGGNUM)
#define _ QStringLiteral

extern MainWindow *w;

int seatTypeTranslate(const QList<QString> &seatTypeStrList, QVector<int> &seatType)
{
    QStringList seatTypeTrans = {
        QStringLiteral("Unknown"),
        QStringLiteral("高级软卧"),
        QStringLiteral("其他"),
        QStringLiteral("软卧"),
        QStringLiteral("软座"),
        QStringLiteral("Unknown"),
        QStringLiteral("无座"),
        QStringLiteral("动卧"),
        QStringLiteral("硬卧"),
        QStringLiteral("硬座"),
        QStringLiteral("二等座"),
        QStringLiteral("一等座"),
        QStringLiteral("商务特等座"),
        QStringLiteral("Unknown")
    };
    int seatTypeTransSize = seatTypeTrans.size();
    for (int i = 0; i < seatTypeStrList.size(); i++) {
        const QString &seatTypeStr = seatTypeStrList[i];
        for (int j = 0; j < seatTypeTransSize; j++) {
            if (seatTypeTrans[j] == seatTypeStr)
                seatType.append(j);
        }
    }

    return seatType.size();
}

QString seatTypeTranslateDesc(int seatType)
{
    QStringList seatTypeTrans = {
        QStringLiteral("Unknown"),
        QStringLiteral("高级软卧"),
        QStringLiteral("其他"),
        QStringLiteral("软卧"),
        QStringLiteral("软座"),
        QStringLiteral("Unknown"),
        QStringLiteral("无座"),
        QStringLiteral("Unknown"),
        QStringLiteral("硬卧"),
        QStringLiteral("硬座"),
        QStringLiteral("二等座"),
        QStringLiteral("一等座"),
        QStringLiteral("商务特等座"),
        QStringLiteral("动卧")
    };
    if (seatType < 0 || seatType >= seatTypeTrans.size())
        return _("Unknown");
    return seatTypeTrans[seatType];
}

QChar seatTypeTransToSubmitCode(int type)
{
    QChar seatTypeSubmitCode[] = {
        '0',
        'A',
        '0',
        '4',
        '0',
        '0',
        'W',
        '0',
        '3',
        '1',
        'O',
        'M',
        '9',
        'F'
    };
    Q_ASSERT(type >= 0 && type < static_cast<int>(sizeof(seatTypeSubmitCode) /
                                  sizeof(seatTypeSubmitCode[0])));
    return seatTypeSubmitCode[type];
}

QChar seatTypeDescTransToSubmitCode(const QString &seatTYpeDesc)
{
    QStringList seatTypeTrans = {
        QStringLiteral("Unknown"),
        QStringLiteral("高级软卧"),
        QStringLiteral("其他"),
        QStringLiteral("软卧"),
        QStringLiteral("软座"),
        QStringLiteral("Unknown"),
        QStringLiteral("无座"),
        QStringLiteral("Unknown"),
        QStringLiteral("硬卧"),
        QStringLiteral("硬座"),
        QStringLiteral("二等座"),
        QStringLiteral("一等座"),
        QStringLiteral("商务特等座"),
        QStringLiteral("动卧")
    };
    QChar seatTypeSubmitCode[] = {
        '0',
        'A',
        '0',
        '4',
        '0',
        '0',
        'W',
        '0',
        '3',
        '1',
        'O',
        'M',
        '9',
        'F'
    };
    for (int i = 0; i < seatTypeTrans.size(); i++) {
        if (seatTYpeDesc == seatTypeTrans[i]) {
            return seatTypeSubmitCode[i];
        }
    }
    return '0';
}

Analysis::Analysis(QVector<QStringList> &trainVec) : trainInfoVec(trainVec)
{

}

Analysis::~Analysis()
{

}

void Analysis::setAnalysisTrain(QVector<QStringList> &trainVec)
{
    trainInfoVec = trainVec;
}

int Analysis::sufficientTicketPrioSelectTrain(const QVariantMap &stationMap)
{
    int trainTicketNum, trainTicketNumMax, ticketMaxidx;
    QVector<int> selectSeatType;
    int selectSeatTypeSize = 0;
    UserData *ud = UserData::instance();
    const QList<QString> &selectedSeatTypeStrList = w->seatTypeDialog->getSelectedSeatType();
    Q_ASSERT(selectedSeatTypeStrList.size() <= TICKETSIZE);
    selectSeatTypeSize = seatTypeTranslate(selectedSeatTypeStrList, selectSeatType);
    const QList<QString> &passengerList = w->passengerDialog->getSelectedPassenger();
    const QSet<QString> &selectedTrainSet = w->trainNoDialog->getSelectedTrainSet();
    bool hasSelectedTrain = false;
    bool subCondition = true;
    trainTicketNumMax = 0;
    ticketMaxidx = -1;

    //qDebug() << selectedTrainSet.values();
again:
    for (int i = 0; i < trainInfoVec.size(); i++) {
        QStringList &trainList = trainInfoVec[i];
        QVector<int> &ticketInfo = trainTicketInfo[i];
        if (trainList.empty() || ticketInfo.empty())
            continue;

        QString trainDesc = _("%1 (%2 %3").arg(trainList[ESTATIONTRAINCODE],
                                               stationMap.value(trainList[EFROMSTATIONTELECODE]).toString(),
                                               stationMap.value(trainList[ETOSTATIONTELECODE]).toString());
        // Currentlly frozened
        if (NetHelper::instance()->queryFrozenTrain(trainDesc)) {
            continue;
        }
        if (!selectedTrainSet.contains(trainDesc))
            continue;
        hasSelectedTrain = true;
        if (subCondition) {
            if (ud->grabSetting.trainPrio.preferGPrio &&
                !trainList[ESTATIONTRAINCODE].isEmpty() &&
                trainList[ESTATIONTRAINCODE][0] != 'G')
                continue;
            if (ud->grabSetting.trainPrio.preferDPrio &&
                !trainList[ESTATIONTRAINCODE].isEmpty() &&
                trainList[ESTATIONTRAINCODE][0] != 'D')
                continue;
            if (ud->grabSetting.trainPrio.preferTimeRangePrio) {
                QStringList timeList = trainList[ESTARTTIME].split(":");
                int trainHour = timeList.size() > 0 ? timeList[0].toInt() : 0;
                int trainMinute = timeList.size() > 1 ? timeList[1].toInt() : 0;
                if (!ud->isTimeInRange(trainHour, trainMinute))
                    continue;
            }
        }
        trainTicketNum = 0;
        for (int j = 0; j < selectSeatTypeSize; j++) {
            if (ticketInfo[selectSeatType[j]] > 0) {
                trainTicketNum += ticketInfo[selectSeatType[j]];
            }
        }
        if (trainTicketNum > trainTicketNumMax) {
            trainTicketNumMax = trainTicketNum;
            ticketMaxidx = i;
        }
    }
    if (hasSelectedTrain && ticketMaxidx == -1 && subCondition) {
        subCondition = false;
        goto again;
    }
    if (!ud->grabSetting.ticketSetting.partialSubmit) {
        return trainTicketNumMax >= passengerList.size() ? ticketMaxidx : -1;
    } else {
        return ticketMaxidx;
    }
}

int Analysis::strictTrainPrioSelectTrain(const QVariantMap &stationMap)
{
    int trainTicketNum;
    QVector<int> selectSeatType;
    int selectSeatTypeSize = 0;
    int partialTicketMax = 0;
    int partialTicketMaxIndex = -1;
    QList<QString> selectedSeatTypeStrList = w->seatTypeDialog->getSelectedSeatType();
    Q_ASSERT(selectedSeatTypeStrList.size() <= TICKETSIZE);
    selectSeatTypeSize = seatTypeTranslate(selectedSeatTypeStrList, selectSeatType);
    const QList<QString> &selectedTrain = w->trainNoDialog->getSelectedTrainList();
    const QList<QString> &passengerList = w->passengerDialog->getSelectedPassenger();
    QString s;
    int j;
    bool partial = UserData::instance()->grabSetting.ticketSetting.partialSubmit;

    for (int i = 0; i < selectedTrain.size(); i++) {
        const QString &trainDesc = selectedTrain[i];
        // Currentlly frozened
        if (NetHelper::instance()->queryFrozenTrain(trainDesc)) {
            continue;
        }
        for (j = 0; j < trainInfoVec.size(); j++) {
            const QStringList &sl = trainInfoVec[j];
            if (trainDesc.startsWith(trainInfoVec[j][ESTATIONTRAINCODE])) {
                s = _("%1 (%2 %3").arg(sl[ESTATIONTRAINCODE],
                                               stationMap.value(sl[EFROMSTATIONTELECODE]).toString(),
                                               stationMap.value(sl[ETOSTATIONTELECODE]).toString());
                if (trainDesc == s)
                    break;
            }
        }
        if (j == trainInfoVec.size())
            continue;

        trainTicketNum = 0;
        for (int k = 0; k < selectSeatTypeSize; k++) {
            if (trainTicketInfo[j][selectSeatType[k]] > 0) {
                trainTicketNum += trainTicketInfo[j][selectSeatType[k]];
            }
        }
        if (trainTicketNum >= passengerList.size()) {
            return j;
        }
        if (partial) {
            if (trainTicketNum > partialTicketMax) {
                partialTicketMax = trainTicketNum;
                partialTicketMaxIndex = j;
            }
        }
    }

    return partialTicketMaxIndex;
}

int Analysis::strictTravelTimePrioSelectTrain(const QVariantMap &stationMap)
{
    int trainTicketNum;
    QVector<int> selectSeatType;
    int selectSeatTypeSize = 0;
    int partialTicketMax = 0;
    int partialTicketMaxIdx = -1;
    int travelTimeShortestIdx = -1;
    const QList<QString> &selectedSeatTypeStrList = w->seatTypeDialog->getSelectedSeatType();
    Q_ASSERT(selectedSeatTypeStrList.size() <= TICKETSIZE);
    selectSeatTypeSize = seatTypeTranslate(selectedSeatTypeStrList, selectSeatType);
    const QSet<QString> &selectedTrainSet = w->trainNoDialog->getSelectedTrainSet();
    const QList<QString> &passengerList = w->passengerDialog->getSelectedPassenger();
    int travelTimeHour;
    int travelTimeMinute;
    int travelTimeShortestHour = 999999;
    int travelTimeShortestMinute = 999999;
    QStringList travelTimeStrList;
    bool partial = UserData::instance()->grabSetting.ticketSetting.partialSubmit;

    for (int i = 0; i < trainInfoVec.size(); i++) {
        QStringList &trainList = trainInfoVec[i];
        if (trainList.empty() || trainTicketInfo[i].empty())
            continue;
        QString trainDesc = _("%1 (%2 %3").arg(trainList[ESTATIONTRAINCODE],
                                               stationMap.value(trainList[EFROMSTATIONTELECODE]).toString(),
                                               stationMap.value(trainList[ETOSTATIONTELECODE]).toString());
        // Currentlly frozened
        if (NetHelper::instance()->queryFrozenTrain(trainDesc)) {
            continue;
        }
        if (!selectedTrainSet.contains(trainDesc))
            continue;
        trainTicketNum = 0;
        for (int j = 0; j < selectSeatTypeSize; j++) {
            if (trainTicketInfo[i][selectSeatType[j]] > 0) {
                trainTicketNum += trainTicketInfo[i][selectSeatType[j]];
            }
        }
        if (trainTicketNum >= passengerList.size()) {
            travelTimeStrList = trainList[ESPENDTIME].split(':', Qt::SkipEmptyParts);
            travelTimeHour = travelTimeStrList.length() > 0 ? travelTimeStrList[0].toInt() : 999999;
            travelTimeMinute = travelTimeStrList.length() > 1 ? travelTimeStrList[1].toInt() : 999999;
            if (travelTimeHour < travelTimeShortestHour ||
                (travelTimeHour == travelTimeShortestHour &&
                travelTimeMinute < travelTimeShortestMinute)) {
                travelTimeShortestHour = travelTimeHour;
                travelTimeShortestMinute = travelTimeMinute;
                travelTimeShortestIdx = i;
            }
        }
        if (partial) {
            if (trainTicketNum > partialTicketMax) {
                partialTicketMax = trainTicketNum;
                partialTicketMaxIdx = i;
            }
        }
    }
    return travelTimeShortestIdx != -1 ? travelTimeShortestIdx : partialTicketMaxIdx;
}

int Analysis::strictStartTimePrioSelectTrain(const QVariantMap &stationMap)
{
    int trainTicketNum;
    QVector<int> selectSeatType;
    int selectSeatTypeSize = 0;
    int partialTicketMax = 0;
    int partialTicketMaxIdx = -1;
    const QList<QString> &selectedSeatTypeStrList = w->seatTypeDialog->getSelectedSeatType();
    Q_ASSERT(selectedSeatTypeStrList.size() <= TICKETSIZE);
    selectSeatTypeSize = seatTypeTranslate(selectedSeatTypeStrList, selectSeatType);
    const QSet<QString> &selectedTrainSet = w->trainNoDialog->getSelectedTrainSet();
    const QList<QString> &passengerList = w->passengerDialog->getSelectedPassenger();
    bool partial = UserData::instance()->grabSetting.ticketSetting.partialSubmit;

    for (int i = 0; i < trainInfoVec.size(); i++) {
        QStringList &trainList = trainInfoVec[i];
        if (trainList.empty() || trainTicketInfo[i].empty())
            continue;
        QString trainDesc = _("%1 (%2 %3").arg(trainList[ESTATIONTRAINCODE],
                                               stationMap.value(trainList[EFROMSTATIONTELECODE]).toString(),
                                               stationMap.value(trainList[ETOSTATIONTELECODE]).toString());
        // Currentlly frozened
        if (NetHelper::instance()->queryFrozenTrain(trainDesc)) {
            continue;
        }
        if (!selectedTrainSet.contains(trainDesc))
            continue;
        trainTicketNum = 0;
        for (int j = 0; j < selectSeatTypeSize; j++) {
            if (trainTicketInfo[i][selectSeatType[j]] > 0) {
                trainTicketNum += trainTicketInfo[i][selectSeatType[j]];
            }
        }
        if (trainTicketNum >= passengerList.size()) {
            return i;
        }
        if (partial) {
            if (trainTicketNum > partialTicketMax) {
                partialTicketMax = trainTicketNum;
                partialTicketMaxIdx = i;
            }
        }
    }

    return partialTicketMaxIdx;
}

void Analysis::initTrainTicketInfo()
{
    trainTicketInfo.resize(trainInfoVec.size());
    QVector<int> trainSeatTypeDataNum = {
                                         ESWZNUM,
                                         EZYNUM,
                                         EZENUM,
                                         EGRNUM,
                                         ERWNUM,
                                         EYBNUM,
                                         EYWNUM,
                                         ERZNUM,
                                         EYZNUM,
                                         EWZNUM,
                                         EQTNUM
    };
    for (int i = 0; i < trainInfoVec.size(); i++) {
        QStringList &trainList = trainInfoVec[i];
        QVector<int> ticketVec;
        ticketVec.resize(TICKETSIZE);
        for (auto &seatTypeNum : trainSeatTypeDataNum) {
            ticketVec[TICKETIDX(seatTypeNum)] = trainList[seatTypeNum] == QStringLiteral("有") ? 100 :
                                                !trainList[seatTypeNum].isEmpty() && trainList[seatTypeNum] != QStringLiteral("无") ?
                                                trainList[seatTypeNum].toInt() : !trainList[seatTypeNum].isEmpty() &&
                                                trainList[seatTypeNum] == _("无") ? 0 : -1;
        }

        trainTicketInfo[i] = ticketVec;
    }
}

void Analysis::sufficientSeatTypePrioSelectSeatType(const QVector<int> &selectSeatType,
                                                  QVector<std::pair<int, int>> &seatVec, int trainNoIdx, int passengerSize)
{
    int ticketSeatMax = 0;
    int ticketSeatTypeMax = 0;
    int selectSeatTypeSize = selectSeatType.size();

    w->formatOutput(_("正在选择席别，使用规则：余票充足的席别优先提交"));
    for (int j = 0; j < selectSeatTypeSize; j++) {
        int ticketNum = trainTicketInfo[trainNoIdx][selectSeatType[j]];
        if (ticketNum > ticketSeatMax) {
            ticketSeatMax = ticketNum;
            ticketSeatTypeMax = selectSeatType[j];
        }
    }
    if (ticketSeatMax) {
        seatVec.push_back(std::pair(ticketSeatTypeMax, ticketSeatMax > passengerSize ?
                                                           passengerSize : ticketSeatMax));
        trainTicketInfo[trainNoIdx][ticketSeatTypeMax] -= ticketSeatMax;
        w->formatOutput(_("已选择席别%1, 余票%2").arg(seatTypeTranslateDesc(ticketSeatTypeMax),
                                                      ticketSeatMax == 100 ?
                                                          _("充足") : _("%1张").arg(ticketSeatMax)));
    }
    // 座位最多的席别余票仍然不足，选择第二个席别（比如二等座，一等座）
    if (ticketSeatMax < passengerSize) {
        int remain = passengerSize - ticketSeatMax;
        for (int j = 0; j < selectSeatTypeSize; j++) {
            int ticketNum = trainTicketInfo[trainNoIdx][selectSeatType[j]];
            if (ticketNum > 0) {
                seatVec.push_back(std::pair(selectSeatType[j], ticketNum > remain ?
                                                                   remain : ticketNum));
                w->formatOutput(_("已选择席别%1, 余票%2").arg(seatTypeTranslateDesc(selectSeatType[j]),
                                                              ticketNum == 100 ?
                                                                  _("充足") : _("%1张").arg(ticketNum)));
                remain -= ticketNum;
                if (remain <= 0) {
                    break;
                }
            }
        }
        // remain还大于0的话说明勾选了余票不足部分提交
        if (remain > 0) {
            w->formatOutput(_("当前车次已选席别余票不足，已开启余票不足自动提交，继续提交..."));
        }
    }
}

void Analysis::strictPriceLowToHighSelectSeatType(const QVector<int> &selectSeatType, QVector<int> priceLowToHigh,
                                                  QVector<std::pair<int, int>> &seatVec, int trainNoIdx, int passengerSize)
{
    int priceLowToHighSize = priceLowToHigh.size();
    int remain = passengerSize;
    int i;
    for (i = 0; i < priceLowToHighSize; i++) {
        if (selectSeatType.contains(priceLowToHigh[i])) {
            int ticketNum = trainTicketInfo[trainNoIdx][priceLowToHigh[i]];
            if (ticketNum > 0) {
                seatVec.push_back(std::pair(priceLowToHigh[i], ticketNum > remain ?
                                                                   remain : ticketNum));
                w->formatOutput(_("已选择席别%1, 余票%2").arg(seatTypeTranslateDesc(priceLowToHigh[i]),
                                                              ticketNum == 100 ?
                                                                  _("充足") : _("%1张").arg(ticketNum)));
                remain -= ticketNum;
                if (remain <= 0) {
                    break;
                }
            }
        }
    }
    // remain还大于0的话说明勾选了余票不足部分提交
    if (remain > 0) {
        w->formatOutput(_("当前车次已选席别余票不足，已开启余票不足自动提交，继续提交..."));
    }
}

QVector<std::pair<int, int>> Analysis::trainSelectSeatType(int trainNoIdx)
{
    UserData *ud = UserData::instance();
    QVector<int> selectSeatType;
    int selectSeatTypeSize = 0;
    const QList<QString> &selectedSeatTypeStrList = w->seatTypeDialog->getSelectedSeatType();
    Q_ASSERT(selectedSeatTypeStrList.size() <= TICKETSIZE);
    selectSeatTypeSize = seatTypeTranslate(selectedSeatTypeStrList, selectSeatType);
    const QList<QString> &passengerList = w->passengerDialog->getSelectedPassenger();
    QVector<std::pair<int, int>> seatVec;

    if (trainNoIdx < trainTicketInfo.size()) {
        if (ud->grabSetting.seatTypePrio.sufficientSeatTypePrio) {
            sufficientSeatTypePrioSelectSeatType(selectSeatType, seatVec, trainNoIdx, passengerList.size());
        } else if (ud->grabSetting.seatTypePrio.strictSeatTypePrio) {
            int remain = passengerList.size();
            for (int j = 0; j < selectSeatTypeSize; j++) {
                int ticketNum = trainTicketInfo[trainNoIdx][selectSeatType[j]];
                if (ticketNum > 0) {
                    seatVec.push_back(std::pair(selectSeatType[j], ticketNum > remain ?
                                                                       remain : ticketNum));
                    remain -= ticketNum;
                    if (remain <= 0) {
                        break;
                    }
                }
            }
            // remain还大于0的话说明勾选了余票不足部分提交
            if (remain > 0) {
                w->formatOutput(_("当前车次已选席别余票不足，已开启余票不足自动提交，继续提交..."));
            }
        } else if (ud->grabSetting.seatTypePrio.priceLowSeatTypePrioWhensufficient) {
            QStringList priceLowToHighStrList = { _("无座"), _("硬座"), _("二等座"),
                                                 _("一等座"), _("软座"), _("硬卧"),
                                                 _("商务特等座"), _("软卧"), _("高级软卧"),
                                                 _("动卧") };
            QVector<int> priceLowToHigh;
            int priceLowToHighSize;
            int i;
            priceLowToHighSize = seatTypeTranslate(priceLowToHighStrList, priceLowToHigh);
            w->formatOutput(_("正在选择席别，使用规则：余票充足时价格低的席别优先提交"));
            for (i = 0; i < priceLowToHighSize; i++) {
                if (selectSeatType.contains(priceLowToHigh[i])) {
                    int ticketNum = trainTicketInfo[trainNoIdx][priceLowToHigh[i]];
                    // 余票充足，即显示为有
                    if (ticketNum == 100) {
                        seatVec.push_back(std::pair(priceLowToHigh[i], passengerList.size()));
                        w->formatOutput(_("已选择席别%1, 余票%2").arg(seatTypeTranslateDesc(priceLowToHigh[i]),
                                                                      ticketNum == 100 ?
                                                                          _("充足") : _("%1张").arg(ticketNum)));
                        break;
                    }
                }
            }
            if (i == priceLowToHighSize) {
                sufficientSeatTypePrioSelectSeatType(selectSeatType, seatVec, trainNoIdx, passengerList.size());
            }
        } else if (ud->grabSetting.seatTypePrio.priceLowSeatTypePrio) {
            QStringList priceLowToHighStrList = { _("无座"), _("硬座"), _("二等座"),
                                                 _("一等座"), _("软座"), _("硬卧"),
                                                 _("商务特等座"), _("软卧"), _("高级软卧"),
                                                 _("动卧") };
            QVector<int> priceLowToHigh;
            seatTypeTranslate(priceLowToHighStrList, priceLowToHigh);
            w->formatOutput(_("正在选择席别，使用规则：价格低的席别优先提交"));
            strictPriceLowToHighSelectSeatType(selectSeatType, priceLowToHigh, seatVec,
                                               trainNoIdx, passengerList.size());
        } else {
            // ud->grabSetting.seatTypePrio.priceHighSeatTypePrio
            QStringList priceHighToLowStrList = { _("动卧"),  _("高级软卧"), _("软卧"),_("商务特等座"),
                                                 _("硬卧"), _("软座"),_("一等座"), _("二等座"), _("硬座"), _("无座")};
            QVector<int> priceHighToLow;
            seatTypeTranslate(priceHighToLowStrList, priceHighToLow);
            w->formatOutput(_("正在选择席别，使用规则：价格高的席别优先提交"));
            strictPriceLowToHighSelectSeatType(selectSeatType, priceHighToLow, seatVec,
                                               trainNoIdx, passengerList.size());
        }
    }
    return seatVec;
}

std::pair<QString, QString> Analysis::generateSubmitTicketInfo(int trainNoIdx, QVector<std::pair<int, int>> &seatVec,
                                                               QVector<QPair<QString, QChar>> &submitSeatType)
{
    const QStringList &passengerList = w->passengerDialog->getSelectedPassenger();
    QString submitTicketStr, oldPassengerTicketStr;
    int j, idx;
    UserData *ud = UserData::instance();

    idx = 0;
    submitSeatType.clear();
    for (int i = 0; i < seatVec.size(); i++) {
        if (i >= passengerList.size())
            break;
        if (idx >= passengerList.size())
            break;
        int k;
        QChar code = seatTypeTransToSubmitCode(seatVec[i].first);
        if (code == '0') {
            qDebug() << "Unsupported seat type: " << seatVec[i].first;
            w->formatOutput(_("不支持的席别类型：") + seatTypeTranslateDesc(seatVec[i].first));
            continue;
        }
        // 无座
        if (code == 'W') {
            code = trainTicketInfo[trainNoIdx][TICKETIDX(EZENUM)] != -1 ? 'O' : '1';  // 二等座 : 硬座
        }
        for (k = 0; k < seatVec[i].second; k++) {
            if (idx >= passengerList.size())
                break;
            for (j = 0; j < ud->passenger.size(); j++) {
                if (ud->passenger[j].passName == passengerList[idx])
                    break;
            }
            if (j >= ud->passenger.size()) {
                idx++;
                continue;
            }
            submitSeatType.append(QPair<QString, QChar>(ud->passenger[j].passName, code));
            submitTicketStr.append(QStringLiteral("%1,0,%2,%3,%4,%5,%6,N,%7_").arg(code, PASSENGERADULT,
                                                                                   ud->passenger[j].passName.toUtf8().toPercentEncoding(),
                                                                                   ud->passenger[j].passIdTypeCode,
                                                                                   ud->passenger[j].passIdNo,
                                                                                   ud->passenger[j].mobile,
                                                                                   ud->passenger[j].allEncStr));
            oldPassengerTicketStr.append(QStringLiteral("%1,%2,%3,%4_").arg(ud->passenger[j].passName.toUtf8().toPercentEncoding(),
                                                                            ud->passenger[j].passIdTypeCode,
                                                                            ud->passenger[j].passIdNo,
                                                                            ud->passenger[j].passType));
            idx++;
        }
    }
    if (submitTicketStr.length() > 0)
        submitTicketStr.truncate(submitTicketStr.length() - 1);
    submitTicketStr.replace(QStringLiteral(","), QStringLiteral("%2C"));
    oldPassengerTicketStr.replace(QStringLiteral(","), QStringLiteral("%2C"));
    w->formatOutput(_("正在生成提交信息..."));

    return std::pair<QString, QString>(submitTicketStr, oldPassengerTicketStr);
}

int Analysis::analysisTrain(std::pair<QString, QString> &ticketStr, QVector<QPair<QString, QChar>> &submitSeatType, const QVariantMap &stationMap)
{
    UserData *ud = UserData::instance();
    int trainNoIdx = -1;
    int methodDescIdx = 0;

    if (trainTicketInfo.isEmpty()) {
        initTrainTicketInfo();
    }
    if (ud->grabSetting.trainPrio.sufficientTicketPrio) {
        trainNoIdx = sufficientTicketPrioSelectTrain(stationMap);
        methodDescIdx = 0;
    } else if (ud->grabSetting.trainPrio.strictTrainPrio) {
        trainNoIdx = strictTrainPrioSelectTrain(stationMap);
        methodDescIdx = 1;
    } else if (ud->grabSetting.trainPrio.travelTimeShortPrio) {
        trainNoIdx = strictTravelTimePrioSelectTrain(stationMap);
        methodDescIdx = 2;
    } else if (ud->grabSetting.trainPrio.strictStartTimePrio) {
        trainNoIdx = strictStartTimePrioSelectTrain(stationMap);
        methodDescIdx = 3;
    }
    if (trainNoIdx != -1) {
        QStringList methodDesc = { _("检测到可预订的已选中车次，使用规则：余票充足的车次优先提交"),
                                   _("检测到可预订的已选中车次，使用规则：按选中车次的顺序提交"),
                                   _("检测到可预订的已选中车次，使用规则：行程时间短的车次优先提交"),
                                   _("检测到可预订的已选中车次，使用规则：按列车发车时间顺序提交")
        };
        QString startStation, endStation;
        w->formatOutput(methodDesc[methodDescIdx]);
        startStation = stationMap.value(trainInfoVec[trainNoIdx][ESTARTSTATIONTELECODE]).toString();
        endStation = stationMap.value(trainInfoVec[trainNoIdx][EENDSTATIONTELECODE]).toString();
        w->formatOutput(_("已选中车次%1 始发站：%2, 终点站：%3, 出发站：%4, 到达站：%5, 出发时间：%6, 到达时间：%7, 历时：%8")
                        .arg(trainInfoVec[trainNoIdx][ESTATIONTRAINCODE],
                            startStation,
                            endStation,
                            stationMap.value(trainInfoVec[trainNoIdx][EFROMSTATIONTELECODE]).toString(),
                            stationMap.value(trainInfoVec[trainNoIdx][ETOSTATIONTELECODE]).toString(),
                            trainInfoVec[trainNoIdx][ESTARTTIME],
                            trainInfoVec[trainNoIdx][EARRIVETIME],
                            trainInfoVec[trainNoIdx][ESPENDTIME]));
        QVector<std::pair<int, int>> seatVec = trainSelectSeatType(trainNoIdx);
        ud->submitTicketInfo.startSTationName = startStation;
        ud->submitTicketInfo.endStationName = endStation;
        ud->submitTicketInfo.fromTime = trainInfoVec[trainNoIdx][ESTARTTIME];
        ud->submitTicketInfo.toTime = trainInfoVec[trainNoIdx][EARRIVETIME];
        ud->submitTicketInfo.travelTime = trainInfoVec[trainNoIdx][ESPENDTIME];
        ticketStr = generateSubmitTicketInfo(trainNoIdx, seatVec, submitSeatType);
    }
    return trainNoIdx;
}

bool Analysis::mayCandidate(const QVariantMap &stationMap, const QString &date)
{
    int ret = -1;
    int i, j;
    int trainInfoVecSize = trainInfoVec.size();
    const QList<QString> &selectedSeatTypeList = w->seatTypeDialog->getSelectedSeatType();
    QVector<int> selectSeatType;
    int selectSeatTypeSize = seatTypeTranslate(selectedSeatTypeList, selectSeatType);
    const QSet<QString> &selectedTrainSet = w->trainNoDialog->getSelectedTrainSet();
    UserData *ud = UserData::instance();
    struct CandidateDateInfo dInfo;

    if (trainTicketInfo.isEmpty()) {
        initTrainTicketInfo();
    }
    for (i = 0; i < trainInfoVecSize; i++) {
        QStringList &trainList = trainInfoVec[i];
        if (trainList[ESECRETSTR].isEmpty()) {
            continue;
        }
        if (trainList[ECANDIDATETRAINFLAG] != _("1")) {
            continue;
        }
        QString trainDesc = _("%1 (%2 %3").arg(trainList[ESTATIONTRAINCODE],
                                               stationMap.value(trainList[EFROMSTATIONTELECODE]).toString(),
                                               stationMap.value(trainList[ETOSTATIONTELECODE]).toString());
        if (!selectedTrainSet.contains(trainDesc)) {
            continue;
        }
        struct CandidateTrainInfo tInfo;
        for (j = 0; j < selectSeatTypeSize; j++) {
            // 其他 and 无座
            if (selectSeatType[j] == 2 ||
                selectSeatType[j] == 6) {
                continue;
            }
            if (trainTicketInfo[i][selectSeatType[j]] == 0) {
                QChar code = seatTypeDescTransToSubmitCode(selectedSeatTypeList[j]);
                if (code != '0') {
                    if (trainList[ECANDIDATESEATLIMIT].contains(code)) {
                        /*w->formatOutput(_("当前%1车次%2席别提交的候补订单较多，可更换车次、席别或稍后重试")
                                            .arg(trainList[ESTATIONTRAINCODE],
                                                 selectedSeatTypeList[j]));*/
                        continue;
                    }
                    tInfo.trainCode = trainList[ESTATIONTRAINCODE];
                    tInfo.secretStr = trainList[ESECRETSTR];
                    tInfo.seatType.append(code);
                    if (!ud->candidateInfo.allSeatType.contains(code)) {
                        ud->candidateInfo.allSeatType.append(code);
                    }
                }
            }
        }
        dInfo.train.append(tInfo);
        dInfo.date = date;
        dInfo.hasUpdate = true;
    }
    if (!dInfo.train.isEmpty()) {
        NetHelper::instance()->candidateEntry(dInfo);
        ret = 0;
    }
    return ret;
}


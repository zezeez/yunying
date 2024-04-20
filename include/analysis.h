#ifndef ANALYSIS_H
#define ANALYSIS_H
#include <QNetworkReply>
#include <QVector>
#include <QString>

class Analysis
{
public:
    explicit Analysis(QVector<QStringList> &trainVec);
    ~Analysis();
    void setAnalysisTrain(QVector<QStringList> &trainVec);
    int sufficientTicketPrioSelectTrain(const QVariantMap &stationMap);
    int strictTrainPrioSelectTrain(const QVariantMap &stationMap);
    int strictTravelTimePrioSelectTrain(const QVariantMap &stationMap);
    int strictStartTimePrioSelectTrain(const QVariantMap &stationMap);
    QVector<std::pair<int, int>> trainSelectSeatType(int trainNoIdx);
    std::pair<QString, QString> generateSubmitTicketInfo(int trainNoIdx, QVector<std::pair<int, int>> &seatVec, QVector<QPair<QString, QChar>> &submitSeatType);
    void initTrainTicketInfo();
    void sufficientSeatTypePrioSelectSeatType(const QVector<int> &selectSeatType,
                                                        QVector<std::pair<int, int>> &seatVec, int trainNoIdx, int passengerSize);
    void strictPriceLowToHighSelectSeatType(const QVector<int> &selectSeatType, QVector<int> priceLowToHigh,
                                                      QVector<std::pair<int, int>> &seatVec, int trainNoIdx, int passengerSize);
    int analysisTrain(std::pair<QString, QString> &ticketStr, QVector<QPair<QString, QChar>> &submitSeatType, const QVariantMap &stationMap);
    bool mayCandidate(const QVariantMap &stationMap, const QString &date);

private:
    QVector<QStringList> &trainInfoVec;
    QVector<QVector<int>> trainTicketInfo;
};

QChar seatTypeDescTransToSubmitCode(const QString &seatTypeDesc);

#endif // ANALYSIS_H

#ifndef SEATDIALOG_H
#define SEATDIALOG_H
#include <QObject>
#include <QDialog>
#include <QVector>
#include <QComboBox>
#include <QLabel>
#include "completeedit.h"

class SeatDialog : public QDialog
{
    Q_OBJECT
public:
    SeatDialog(QWidget *parent = nullptr);
    ~SeatDialog();
    void showSeatType(int index);
    void updateSelectedTips();
    void clearSelected();
    QString getChoosedSeats(QChar seatType);

private:
    QVector<QPair<QChar, QVector<int>>> seatsVec;
    QComboBox *seatTypeCB;
    QVector<QLabel *> seatDescLB;
    QVector<ClickLabel *> seatVCL;
    QVector<bool> seatSelected;
};

#endif // SEATDIALOG_H

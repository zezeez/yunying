#ifndef SEATTYPEDIALOG_H
#define SEATTYPEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QListWidget>
#include <QPushButton>

class SeatTypeDialog : public QDialog
{
    Q_OBJECT
public:
    SeatTypeDialog(QWidget *parent = nullptr);
    ~SeatTypeDialog();
    void setUp();
    void updateSelectedTips();
    const QList<QString> &getSelectedSeatType() const;
    void setSelectedSeatType();
    void setUnselectedSeatType();
    void clearSelectedSeatType();
    void clearUnSelectedSeatType();
    void moveUpSeatType();
    void moveDownSeatType();
    void enterGrabTicketMode();
    void exitGrabTicketMode();
    QString seatTypeCodeToName(QChar code);

public Q_SLOTS:
    void show();
    void hide();

public:
    QDialog *dialog;
    QListWidget *selected;
    QListWidget *unSelected;
    QPushButton *addSelectedPb;
    QPushButton *addUnSelectedPb;
    QPushButton *addSelectedAllPb;
    QPushButton *addUnSelectedAllPb;
    QPushButton *moveUpSelectedPb;
    QPushButton *moveDownSelectPb;
};

#endif // SEATTYPEDIALOG_H

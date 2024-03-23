#ifndef PASSENGERDIALOG_H
#define PASSENGERDIALOG_H
#include <QDialog>
#include <QString>
#include <QListWidget>
#include <QPushButton>

class PassengerDialog : public QDialog
{
    Q_OBJECT
public:
    PassengerDialog(QWidget *parent = nullptr);
    ~PassengerDialog();
    void setUp();
    void refreshPassengerInfo();
    void updateSelectedTips();
    const QList<QString> &getSelectedPassenger() const;
    void setSelectedPassenger();
    void setUnselectedPassenger();
    void addSelectedPassenger(const QString &passengerName);
    void addUnSelectedPassenger(const QString &passengerName);
    void clearUnSelectedPassenger();
    void clearSelectedPassenger();
    void clearPassenger();
    void moveUpPassenger();
    void moveDownPassenger();
    void enterGrabTicketMode();
    void exitGrabTicketMode();

public:
    QListWidget *selected;
    QListWidget *unSelected;
    QPushButton *addSelectedPb;
    QPushButton *addUnSelectedPb;
    QPushButton *addSelectedAllPb;
    QPushButton *addUnSelectedAllPb;
    QPushButton *moveUpSelectedPb;
    QPushButton *moveDownSelectPb;
    QPushButton *refreshPassengerPb;
};

#endif // PASSENGERDIALOG_H

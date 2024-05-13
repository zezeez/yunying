#ifndef TRAINNODIALOG_H
#define TRAINNODIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QListWidget>
#include <QPushButton>
#include <QSet>

class TrainNoDialog : public QDialog
{
    Q_OBJECT
public:
    TrainNoDialog(QWidget *parent = nullptr);
    ~TrainNoDialog();
    void setUp();
    void updateSelectedTips();
    bool hasTrain(const QString &trainInfo);
    void addTrain(const QString &trainInfo, const QString &display);
    void addTrainFinish();
    void addSelectedTrain(const QString &trainInfo);
    void removeSelectedTrain(const QString &trainInfo);
    void addSelectedTrainAll();
    void clearSelectedTrain();
    void clearUnSelectedTrain();
    const QList<QString> &getSelectedTrainList() const;
    const QSet<QString> &getSelectedTrainSet() const;
    const QSet<QString> &getAllTrainSet() const;
    void setSelectedTrainNo();
    void setUnselectedTrainNo();
    void moveUpTrain();
    void moveDownTrain();
    void enterGrabTicketMode();
    void exitGrabTicketMode();

public:
    QListWidget *selected;
    QListWidget *unSelected;
    QPushButton *addSelectedPb;
    QPushButton *addUnSelectedPb;
    QPushButton *addSelectedAllPb;
    QPushButton *addUnSelectedAllPb;
    QSet<QString> trainSet;
    QSet<QString> selectedTrainSet;
    QPushButton *moveUpSelectedPb;
    QPushButton *moveDownSelectPb;
};

#endif // TRAINNODIALOG_H

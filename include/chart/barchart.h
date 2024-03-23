#ifndef BARCHART_H
#define BARCHART_H
#include <QDialog>
#include <QVector>
#include <QBarSet>
#include <QBarSeries>
#include <QChart>
#include <QChartView>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QQueue>

class BarChartView : public QDialog
{
    Q_OBJECT
public:
    BarChartView(QWidget *parent = nullptr);
    ~BarChartView();
    void update(QVector<int> &data);
private:
    QVector<QBarSet *> dataSet;
    QBarSeries *series;
    QBarCategoryAxis *axisX;
    QValueAxis *axisY;
    QChart *chart;
    QChartView *view;
    int valueMax;
    int valueMaxIndex;
};

#endif // BARCHART_H

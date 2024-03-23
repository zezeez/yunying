#ifndef LINECHART_H
#define LINECHART_H
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QDialog>
#include <QValueAxis>

class LineChartView : public QDialog
{
public:
    LineChartView(QWidget *parent = nullptr);
    ~LineChartView();
    void update(int d);
private:
    QLineSeries *series;
    QChart *chart;
    QChartView *view;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QVector<QPointF> data;
    int valueMax;
    int valueMaxIndex;
};

#endif // LINECHART_H

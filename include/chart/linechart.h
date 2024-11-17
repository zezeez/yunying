#ifndef LINECHART_H
#define LINECHART_H
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QDialog>
#include <QValueAxis>

class LineChartView : public QDialog
{
    Q_OBJECT
public:
    LineChartView(QWidget *parent = nullptr);
    ~LineChartView();
    void setTitle(const QString &title);
    void setXSeriesTitle(const QString &title);
    void setYSeriesTitle(const QString &title);
    void legendHide();
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

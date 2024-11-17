#include <QVBoxLayout>
#include <QLegendMarker>
#include "chart/linechart.h"

#define _ QStringLiteral

LineChartView::LineChartView(QWidget *parent) : QDialog(parent)
{
    series = new QLineSeries;
    chart = new QChart;

    valueMax = 0;
    valueMaxIndex = 0;

    for (int i = 0; i < 9; i++) {
        data << QPointF(i, 0);
    }

    *series << data;
    chart->addSeries(series);

    chart->createDefaultAxes();
    QList<QAbstractAxis *> absSeries = chart->axes(Qt::Horizontal);
    if (!absSeries.isEmpty()) {
        absSeries[0]->setRange(0, 8);
        absSeries[0]->setGridLineVisible(false);
        static_cast<QValueAxis *>(absSeries[0])->setLabelFormat("%u");
    }
    absSeries = chart->axes(Qt::Vertical);
    if (!absSeries.isEmpty()) {
        absSeries[0]->setRange(0, 100);
        absSeries[0]->setGridLineVisible(false);
        static_cast<QValueAxis *>(absSeries[0])->setLabelFormat("%u");
    }

    view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(view);
    setLayout(vlayout);
    resize(600, 400);
}

LineChartView::~LineChartView()
{

}

void LineChartView::setTitle(const QString &title)
{
    chart->setTitle(title);
}

void LineChartView::setXSeriesTitle(const QString &title)
{
    QList<QAbstractAxis *> absSeries = chart->axes(Qt::Horizontal);
    if (!absSeries.isEmpty()) {
        absSeries[0]->setTitleText(title);
    }
}

void LineChartView::setYSeriesTitle(const QString &title)
{
    QList<QAbstractAxis *> absSeries = chart->axes(Qt::Vertical);
    if (!absSeries.isEmpty()) {
        absSeries[0]->setTitleText(title);
    }
}

void LineChartView::legendHide()
{
    chart->legend()->hide();
}

void LineChartView::update(int d)
{
    series->clear();
    for (int i = data.size() - 1; i > 0; i--) {
        data[i].setY(data[i - 1].y());
    }
    data[0].setY(d);
    *series << data;

    if (d > valueMax) {
        valueMax = d;
    } else {
        valueMaxIndex++;
        if (valueMaxIndex > data.size()) {
            valueMax = 0;
            valueMaxIndex = 0;
            for (int i = 0; i < data.size(); i++) {
                if (data[i].y() > valueMax) {
                    valueMax = data[i].y();
                    valueMaxIndex = i;
                }
            }
        }
    }
    QList<QAbstractAxis *> absSeries = chart->axes(Qt::Vertical);
    if (!absSeries.isEmpty()) {
        absSeries[0]->setRange(0, valueMax > 100 ? valueMax + 1 : 100);
    }
    //view->update();
}

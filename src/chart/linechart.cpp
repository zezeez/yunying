#include <QVBoxLayout>
#include "chart/linechart.h"

#define _ QStringLiteral

LineChartView::LineChartView(QWidget *parent) : QDialog(parent)
{
    series = new QLineSeries;
    chart = new QChart;

    valueMax = 0;
    valueMaxIndex = 0;

    for (int i = 0; i < 6; i++) {
        data << QPointF(i + 0.5, 0) << QPointF(i + 1, 0);
    }

    *series << data;
    chart->addSeries(series);

    chart->createDefaultAxes();
    QList<QAbstractAxis *> absSeries = chart->axes(Qt::Horizontal);
    if (!absSeries.isEmpty()) {
        absSeries[0]->setTitleText(_("过去/小时"));
        absSeries[0]->setRange(0.5, 6);
        absSeries[0]->setGridLineVisible(false);
        static_cast<QValueAxis *>(absSeries[0])->setLabelFormat("%u");
    }
    absSeries = chart->axes(Qt::Vertical);
    if (!absSeries.isEmpty()) {
        absSeries[0]->setTitleText(_("时间/ms"));
        absSeries[0]->setRange(0, 100);
        absSeries[0]->setGridLineVisible(false);
        static_cast<QValueAxis *>(absSeries[0])->setLabelFormat("%u");
    }
    chart->setTitle(_("时延统计"));

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
        if (valueMaxIndex > 11) {
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

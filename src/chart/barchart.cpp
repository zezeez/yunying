#include "chart/barchart.h"
#include <QVBoxLayout>
#include "nethelper.h"

#define _ QStringLiteral

BarChartView::BarChartView(QWidget *parent) : QDialog(parent)
{
    QStringList desc = {
        _("提交"),
        _("失败提交"),
        _("候补"),
        _("失败候补"),
        _("网络异常"),
        _("错误应答"),
    };
    valueMax = 0;
    valueMaxIndex = 0;

    dataSet.resize(desc.size());
    series = new QBarSeries;
    for (int i = 0; i < dataSet.size(); i++) {
        dataSet[i] = new QBarSet(desc[i]);
        series->append(dataSet[i]);
    }
    chart = new QChart;
    chart->addSeries(series);
    chart->setTitle(_("异常统计"));
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList hours = {
        _("1"),
        _("2"),
        _("3"),
        _("4"),
        _("5"),
        _("6"),
        _("7"),
        _("8"),
        _("9"),
        _("10"),
        _("11"),
        _("12"),
    };
    for (int i = 0; i < dataSet.size(); i++) {
        for (int j = 0; j < hours.size(); j++) {
            *dataSet[i] << 0;
        }
    }
    axisX = new QBarCategoryAxis;
    axisX->append(hours);
    axisX->setTitleText(_("过去/小时"));
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis;
    axisY->setRange(0, 30);
    axisY->setLabelFormat(_("%d"));
    axisY->setTitleText(_("次数"));
    axisY->setGridLineVisible(false);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(view);
    setLayout(vlayout);
    resize(600, 400);
}

BarChartView::~BarChartView()
{

}

void BarChartView::update(QVector<int> &d)
{
    QStringList desc = {
        _("提交"),
        _("失败提交"),
        _("候补"),
        _("失败候补"),
        _("网络异常"),
        _("错误应答"),
    };
    if (d.size() < desc.size()) {
        return;
    }

    bool setMax = false;
    for (int i = 0; i < desc.size(); i++) {
        dataSet[i]->remove(11);
        dataSet[i]->insert(0, d[i]);
        if (d[i] > valueMax) {
            valueMax = d[i];
            valueMaxIndex = 0;
            setMax = true;
        }
    }
    if (!setMax) {
        valueMaxIndex++;
        if (valueMaxIndex > 11) {
            valueMax = 0;
            valueMaxIndex = 0;
            for (int i = 0; i < desc.size(); i++) {
                for (int j = 0; j < 12; j++) {
                    if (dataSet[i]->at(j) > valueMax) {
                        valueMax = dataSet[i]->at(j);
                        valueMaxIndex = j;
                    }
                }
            }
        }
    }
    axisY->setRange(0, valueMax > 10 ? valueMax + 1 : 10);
    //view->update();
}

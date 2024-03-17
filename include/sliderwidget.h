#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSlider>

class SliderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SliderWidget(QWidget *parent = 0);
    static bool state;

private:
    QSlider *varificationSlider;//滑动条
    QLabel *varificationLab1,*varificationLab2;//用于放置图片

signals:

public slots:
    void imageMove();
    void imageStop();
};

#endif // SLIDERWIDGET_H

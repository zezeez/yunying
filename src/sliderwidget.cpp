#include "sliderwidget.h"
#include <QIcon>
#include <QDebug>
#include <QMessageBox>

bool SliderWidget::state = false;

SliderWidget::SliderWidget(QWidget *parent) : QWidget(parent)
{
    this->setWindowTitle("安全验证");
    this->setWindowIcon(QIcon(":/image/提示.png"));
    this->setFixedSize(420,400);
    this->setWindowModality(Qt::ApplicationModal);//阻塞除当前窗体外的其他所有窗体
    //滑动条
    varificationSlider=new QSlider(this);
    varificationSlider->setOrientation(Qt::Horizontal);
    varificationSlider->setRange(0,310);//设置滑动条value的范围
    varificationSlider->setGeometry(20,360,380,20);
    //滑动条样式表设计
    varificationSlider->setStyleSheet("QSlider:handle{border-image:url(:/image/Slider.png);width: 15px;}"
                                      "QSlider:handle:hover{border-image:url(:/image/Slider_hover.png);width: 15px;}"
                                      "QSlider:groove{border: 1px solid #999999;}"
                                      "QSlider:add-page{background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 #bcbcbc, "
                                      "stop:0.25 #bcbcbc, stop:0.5 #bcbcbc, stop:1 #bcbcbc);}"
                                      "QSlider:sub-page{background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 #D1E9FE, "
                                      "stop:0.25 #D1E9FE, stop:0.5 #D1E9FE, stop:1 #D1E9FE);}");
    connect(varificationSlider,SIGNAL(sliderMoved(int)),this,SLOT(imageMove()));//滑块移动触发信号
    connect(varificationSlider,SIGNAL(valueChanged(int)),this,SLOT(imageMove()));//滑动条的值发生改变触发信号
    connect(varificationSlider,SIGNAL(sliderReleased()),this,SLOT(imageStop()));//鼠标释放触发信号

    varificationLab1=new QLabel(this);
    varificationLab1->setPixmap(QPixmap(":/image/noon.jpg").scaled(380,292));//最大的图片
    varificationLab1->setGeometry(20,50,380,292);

    varificationLab2=new QLabel(this);
    varificationLab2->setPixmap(QPixmap(":/image/slider_noon.png").scaled(66,66));//缺口图片
    varificationLab2->setGeometry(25,240,66,66);
    varificationLab2->raise();

    //验证提示
    QLabel *tipLab=new QLabel("拖动下方滑块完成拼图",this);
    tipLab->setGeometry(20,0,350,50);
    tipLab->setStyleSheet("font:14pt; font-family:'微软雅黑'");
}

//缺块图片移动槽函数
void SliderWidget::imageMove()
{
    //控制缺块图片水平移动
    this->varificationLab2->move(25+this->varificationSlider->value(),this->varificationLab2->y());
}

//鼠标释放进行验证
void SliderWidget::imageStop()
{
    int ret=this->varificationSlider->value();
    //滑动条的值在某个范围内（可自行设定）  对应缺块图片正好拼接上原图
    if(ret>=252 && ret<=257)
    {
        SliderWidget::state = true;
        QMessageBox msgBox;
        msgBox.setStyleSheet("QMessageBox QLabel{min-width: 350px; "
                             "min-height: 100px;font:16pt; font-family:'楷体';}");
        msgBox.setText("验证成功！");
        msgBox.setWindowTitle("安全验证");
        msgBox.setWindowIcon(QIcon(":/image/提示.png"));
        msgBox.exec();
        this->varificationSlider->setValue(0);
        this->varificationLab2->move(25+this->varificationSlider->value(),this->varificationLab2->y());
        this->close();
    }
    else  //不在这个范围内  则滑块和缺块图片回归原位
    {
        this->varificationSlider->setValue(0);
        this->varificationLab2->move(25+this->varificationSlider->value(),this->varificationLab2->y());
    }
}

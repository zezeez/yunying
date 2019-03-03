#include "completeedit.h"
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QDebug>
#include <QPainter>

CompleteEdit::CompleteEdit(QWidget *parent)
    : QLineEdit(parent), c(nullptr)
{
}

CompleteEdit::~CompleteEdit()
{
}

void CompleteEdit::setCompleter(InputCompleter *completer)
{
    if (c) {
        QObject::disconnect(c, nullptr, this, nullptr);
        delete c;
    }

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    connect(completer, SIGNAL(activated(const QString &)), this, SLOT(insertCompletion(const QString &)));
}

InputCompleter *CompleteEdit::completer() const
{
    return c;
}

void CompleteEdit::insertCompletion(const QString &completion)
{
    setText(completion.split(' ').at(0));
    selectAll();
}

void CompleteEdit::keyPressEvent(QKeyEvent *e)
{
    if (c && c->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // Let the completer do default behavior
        }
    }

    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!isShortcut)
        QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (!c)
        return;

    bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
    {
        c->popup()->hide();
        return;
    }

    c->update(text());
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
}

void VarCodeLabel::mouseMoveEvent(QMouseEvent *ev)
{
    QLabel::mouseMoveEvent(ev);
}

VarCodeLabel::VarCodeLabel(QWidget *parent) :
    QLabel(parent), area(9)
{
    QVector<mapArea>::iterator it;
    for (it = area.begin(); it != area.end(); ++it) {
        (*it).active = false;
        (*it).selected = false;
    }

    for (int i = 0; i < 9; i++) {
        picLabel[i].setParent(this);
        picLabel[i].hide();
        picLabel[i].setFixedSize(64, 64);
        picLabel[i].setPixmap(QPixmap(QStringLiteral(":/icon/images/selected.svg")));
    }
}

void VarCodeLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        QPoint point;
        //point.setX(ev->x() + 10);
        //point.setY(ev->y() - 35);
        //point = ev->pos();
        point.setX(ev->x() + 20);
        point.setY(ev->y() - 35);
        int ar = getArea(ev->x(), ev->y());
        if (ar > 0 && ar <= 8) {
            struct mapArea &ma = area[ar];
            ma.pos = point;
            if (!picLabel[ar].isVisible()) {
                picLabel[ar].move(ev->x() - 30, ev->y() - 45);
                picLabel[ar].show();
            } else {
                picLabel[ar].hide();
            }
            ma.selected = !ma.selected;
        }
        qDebug() << "(x, y) = " << "(" << point.rx() << ", " << point.ry() << ")" << endl;
    }
}

void VarCodeLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);
}

void VarCodeLabel::paintEvent(QPaintEvent *ev)
{
    QLabel::paintEvent(ev);
}

int VarCodeLabel::getArea(int x, int y)
{
    int w = width();
    int h = height();
    int xstep = w / 4;
    int ystep = (h - 45) / 2;
    int i, j;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            int xmax = xstep * (j + 1);
            int xmin = xstep * j;
            int ymax = ystep * (i + 1);
            int ymin = ystep * i;
            if (x >= xmin && x < xmax && y >= ymin + 45 && y < ymax + 42) {
                return i * 4 + j + 1;
            }
        }
    }
    return 0;
}

void VarCodeLabel::clearSelected()
{
    for (int i = 0; i < area.size(); i++) {
        area[i].selected = false;
    }
    for (int i = 0; i < 9; i++) {
        picLabel[i].hide();
    }
}

QVector<mapArea> &VarCodeLabel::getPoints()
{
    return area;
}

VarCodeLabel::~VarCodeLabel()
{

}

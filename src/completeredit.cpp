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
    if (c)
        QObject::disconnect(c, nullptr, this, nullptr);

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
    QLabel(parent)
{
    pixMap.load(QStringLiteral(":/icon/images/selected.png"));
}

void VarCodeLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        QPoint point;
        //point.setX(ev->x() + 10);
        //point.setY(ev->y() - 35);
        point = ev->pos();
        points.push_back(point);
        update();
        qDebug() << "(x, y) = " << "(" << ev->x() << ", " << ev->y() << ")" << endl;
    }
}

void VarCodeLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);
}

void VarCodeLabel::paintEvent(QPaintEvent *ev){
    QPainter painter(this);
    QVector<QPoint>::const_iterator it;

    for (it = points.cbegin(); it != points.cend(); ++it) {
        QPoint point = *it;
        painter.drawPixmap(point.rx(), point.ry(), 60, 60, pixMap);
    }
    points.clear();
    QLabel::paintEvent(ev);
}

VarCodeLabel::~VarCodeLabel()
{

}

#include "completeedit.h"
#include <QAbstractItemView>
#include <QKeyEvent>

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

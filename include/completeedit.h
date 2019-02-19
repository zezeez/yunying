#ifndef COMPLETEEDIT_H
#define COMPLETEEDIT_H

#include <QLineEdit>
#include <QStringList>
#include <QStringListModel>
#include <QString>
#include <QCompleter>
#include <QStack>
#include <QLabel>
#include <QPixmap>

class InputCompleter : public QCompleter
{
    Q_OBJECT

public:
    inline InputCompleter(const QMap<QString, QStringList> &words, QObject *parent) :
            QCompleter(parent), m_list(words), m_model(), isAppend(true)
    {
        setModel(&m_model);
    }

    void update(QString word)
    {
        // Do any filtering you like.
        QMap<QString, QStringList>::const_iterator it;
        QMap<QString, QStringList> previous, current;
        QStringList result;

        int count = word.length() - m_word.length();
        if (!keyWordStack.isEmpty()) {
            if (count > 0) {
                isAppend = true;
                previous = keyWordStack.top();
                for (it = previous.cbegin(); it != previous.cend(); it++) {
                    if (it.key().startsWith(word)) {
                        result << it.value().at(1) + " " + it.value().at(3);
                        current.insertMulti(it.key(), it.value());
                    }
                }
                keyWordStack.push(current);
            } else {
                if (isAppend)
                    count--;
                isAppend = false;
                while (count++ < 0) {
                    if (!keyWordStack.isEmpty())
                        current = keyWordStack.pop();
                }
                for (it = current.cbegin(); it != current.cend(); it++) {
                    result << it.value().at(1) + " " + it.value().at(3);
                }
            }
        } else {
            for (it = m_list.cbegin(); it != m_list.cend(); it++) {
                if (it.key().startsWith(word)) {
                    result << it.value().at(1) + " " + it.value().at(3);
                    current.insertMulti(it.key(), it.value());
                }
            }
            keyWordStack.push(current);
        }

        m_model.setStringList(result);
        m_word = word;
        complete();
    }

    inline QString word()
    {
        return m_word;
    }

private:
    QMap<QString, QStringList> m_list;
    QStack<QMap<QString, QStringList>> keyWordStack;
    QStringListModel m_model;
    QString m_word;
    bool isAppend;
};

class CompleteEdit : public QLineEdit
{
    Q_OBJECT

public:
    CompleteEdit(QWidget *parent = nullptr);
    ~CompleteEdit();

    void setCompleter(InputCompleter *c);
    InputCompleter *completer() const;

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void insertCompletion(const QString &completion);

private:
    InputCompleter *c;
};

class VarCodeLabel : public QLabel
{
    Q_OBJECT
public:
    VarCodeLabel(QWidget *parent = nullptr);
    ~VarCodeLabel();

    QVector<QPoint> &getPoints()
    {
        return points;
    }
protected:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    QPixmap pixMap;
    QVector<QPoint> points;
};

#endif // COMPLETEEDIT_H

#ifndef COMPLETEEDIT_H
#define COMPLETEEDIT_H

#include <QLineEdit>
#include <QStringList>
#include <QStringListModel>
#include <QString>
#include <QCompleter>

class InputCompleter : public QCompleter
{
    Q_OBJECT

public:
    inline InputCompleter(const QMap<QString, QStringList> &words, QObject *parent) :
            QCompleter(parent), m_list(words), m_model()
    {
        setModel(&m_model);
    }

    inline void update(QString word)
    {
        // Do any filtering you like.
        // Here we just include all items that contain word.
        QMap<QString, QStringList>::const_iterator it;
        QStringList result;

        for (it = m_list.cbegin(); it != m_list.cend(); it++) {
            if (it.key().startsWith(word)) {
                result << it.value().at(1) + " " + it.value().at(3);
            }
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
    QStringListModel m_model;
    QString m_word;
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

#endif // COMPLETEEDIT_H

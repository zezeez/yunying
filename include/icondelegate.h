#ifndef ICONDELEGATE_H
#define ICONDELEGATE_H
#include <QTableView>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <userdata.h>

#define _ QStringLiteral

class IconDelegate : public QStyledItemDelegate
{
public:
    IconDelegate(QObject* parent = nullptr) :
        QStyledItemDelegate(parent)
    {
        fu.load(_(":/icon/images/fu.ico"));
        zi.load(_(":/icon/images/zi.ico"));
        dong.load(_(":/icon/images/dong.ico"));
        start.load(_(":/icon/images/start.ico"));
        pass.load(_(":/icon/images/pass.ico"));
        end.load(_(":/icon/images/end.ico"));
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        bool isFu = false;
        bool isZi = false;
        bool isDong = false;
        bool isStart = false, isEnd = false;
        bool handled = false;
        int type = 0;
        QString text;
        QRect rect;
        QPen pen(QColor(99, 184, 255), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QFont font(_("Times"), QFont::Black);

        if (option.state & QStyle::State_Selected) {
            pen.setColor(Qt::white);
        }

        switch (index.column()) {
        case EFROMSTATIONCOL:
            isStart = index.data(Qt::DecorationRole).toBool();
            text = index.data(Qt::ToolTipRole).toString();
            rect = option.rect;
            painter->save();
            if (isStart) {
                rect.setWidth(start.width());
                rect.setHeight(start.height());
                painter->drawPixmap(rect, start);
            } else {
                rect.setWidth(pass.width());
                rect.setHeight(pass.height());
                painter->drawPixmap(rect, pass);
            }
            painter->setPen(pen);
            font.setPointSize(10);
            font.setBold(true);
            painter->setFont(font);
            rect = painter->boundingRect(rect.right() + 1, rect.y(),
                                         option.rect.width() - start.width(),
                                         option.rect.height(), Qt::AlignCenter, text);
            painter->drawText(rect, text);
            //painter->drawText(rect.right() + 1, rect.bottom(), fromName);
            painter->restore();
            break;
        case ETOSTATIONCOL:
            isEnd = index.data(Qt::DecorationRole).toBool();
            text = index.data(Qt::ToolTipRole).toString();
            rect = option.rect;
            painter->save();
            if (isEnd) {
                rect.setWidth(end.width());
                rect.setHeight(end.height());
                painter->drawPixmap(rect, end);
            } else {
                rect.setWidth(pass.width());
                rect.setHeight(pass.height());
                painter->drawPixmap(rect, pass);
            }
            painter->setPen(pen);
            font.setPointSize(10);
            font.setBold(true);
            painter->setFont(font);
            rect = painter->boundingRect(rect.right() + 1, rect.y(),
                                         option.rect.width() - start.width(),
                                         option.rect.height(), Qt::AlignCenter, text);
            painter->drawText(rect, text);
            //painter->drawText(rect.right() + 1, rect.bottom(), fromName);
            painter->restore();
            break;
        case EOTHERCOL:
            type = index.data(Qt::UserRole).toInt();
            isFu = type & (1 << 0);
            isZi = type & (1 << 1);
            isDong = type & (1 << 2);
            if (isFu) {
                painter->save();
                rect = option.rect;
                rect.setLeft(rect.left());
                rect.setWidth(fu.width());
                painter->drawPixmap(rect, fu);
                painter->restore();
                handled = true;
            }
            if (isZi) {
                //QPixmap pixmap2 = zi.scaled(option.rect.width() / 2, option.rect.height(), Qt::KeepAspectRatio);
                painter->save();
                int l1[2] = { 0, fu.width() };
                rect = option.rect;
                int left = 0;
                left += rect.left();
                left += l1[isFu];
                rect.setLeft(left);
                rect.setWidth(zi.width());
                painter->drawPixmap(rect, zi);
                painter->restore();
                handled = true;
            }
            if (isDong) {
                painter->save();
                rect = option.rect;
                int l1[2] = { 0, fu.width() };
                int l2[2] = { 0, zi.width() };
                int left = 0;
                left += rect.left();
                left += l1[isFu];
                left += l2[isZi];
                rect.setLeft(left);
                rect.setWidth(dong.width());
                painter->drawPixmap(rect, dong);
                painter->restore();
                handled = true;
            }
            if (!handled) {
                QStyledItemDelegate::paint(painter, option, index);
            }
            break;
        default:
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        //QPixmap pixmap = fu.scaled(option.rect.width() / 2, option.rect.height(), Qt::KeepAspectRatio);
    }
private:
    QPixmap fu;
    QPixmap zi;
    QPixmap dong;
    QPixmap start;
    QPixmap pass;
    QPixmap end;
};

#endif // ICONDELEGATE_H
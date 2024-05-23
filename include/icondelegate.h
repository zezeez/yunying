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
        bool trunc = false;
        int type = 0;
        int width, textWidth;
        int spendDays;
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
            painter->setRenderHint(QPainter::Antialiasing);  //抗锯齿
            painter->setRenderHints(QPainter::SmoothPixmapTransform);  //平滑像素图变换
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
            width = option.rect.width() - start.width();
            textWidth = text.size() << 4;
            while (!text.isEmpty() && width < textWidth) {
                text.truncate(text.size() - 1);
                textWidth -= 16;
                trunc = true;
            }
            if (trunc) {
                text += _("..");
            }
            rect = painter->boundingRect(rect.right() + 1, rect.y(),
                                         width,
                                         option.rect.height(), Qt::AlignCenter, text);
            painter->drawText(rect, text);
            painter->restore();
            break;
        case ETOSTATIONCOL:
            isEnd = index.data(Qt::DecorationRole).toBool();
            text = index.data(Qt::ToolTipRole).toString();
            rect = option.rect;
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);  //抗锯齿
            painter->setRenderHints(QPainter::SmoothPixmapTransform);  //平滑像素图变换
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
            width = option.rect.width() - start.width();
            textWidth = text.size() << 4;
            while (!text.isEmpty() && width < textWidth) {
                text.truncate(text.size() - 1);
                textWidth -= 16;
                trunc = true;
            }
            if (trunc) {
                text += _("..");
            }
            rect = painter->boundingRect(rect.right() + 1, rect.y(),
                                         width,
                                         option.rect.height(), Qt::AlignCenter, text);

            painter->drawText(rect, text);
            painter->restore();
            break;
        case EARRIVETIMECOL:
            text = index.data(Qt::DisplayRole).toString();
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setRenderHints(QPainter::SmoothPixmapTransform);
            if (pen.color() != Qt::white) {
                pen.setColor(QColor(205, 104, 137));
            }
            painter->setPen(pen);
            rect = painter->boundingRect(option.rect.x(), option.rect.y(),
                                         option.rect.width(),
                                         option.rect.height(), Qt::AlignCenter, text);
            painter->drawText(rect, text);

            spendDays = index.data(Qt::UserRole).toInt();
            if (spendDays > 0) {
                font.setPointSize(7);
                painter->setFont(font);
                text = _("+%1").arg(spendDays);
                rect = painter->boundingRect(option.rect.x() + option.rect.width() - 18,
                                             option.rect.y() - 5,
                                             20,
                                             20,
                                             Qt::AlignCenter,
                                             text);
                painter->drawText(rect, text);
            }
            painter->restore();
            break;
        case EOTHERCOL:
            type = index.data(Qt::UserRole).toInt();
            isFu = ISFUXING(type);
            isZi = ISZINENG(type);
            isDong = ISDONGGAN(type);
            if (isFu) {
                painter->save();
                //QPixmap pixmap2 = fu.scaled(option.rect.width() / 2, option.rect.height(), Qt::KeepAspectRatio);
                rect = option.rect;
                rect.setLeft(rect.left());
                //rect.setTopLeft(QPoint(rect.x(), rect.y() + (rect.height() - fu.height() + 8) / 2));
                rect.setTopLeft(QPoint(rect.x(), rect.y() + 2));
                rect.setWidth(fu.width() / 2);
                rect.setHeight(option.rect.height() - 4);
                painter->setRenderHint(QPainter::Antialiasing);  //抗锯齿
                painter->setRenderHints(QPainter::SmoothPixmapTransform);  //平滑像素图变换
                painter->drawPixmap(rect, fu);
                painter->restore();
                handled = true;
            }
            if (isZi) {
                //QPixmap pixmap2 = zi.scaled(option.rect.width() / 2, option.rect.height(), Qt::KeepAspectRatio);
                painter->save();
                int l1[2] = { 0, fu.width() / 2 + 4 };
                rect = option.rect;
                int left = 0;
                left += rect.left();
                left += l1[isFu];
                rect.setLeft(left);
                rect.setTopLeft(QPoint(rect.x(), rect.y() + 2));
                rect.setWidth(zi.width() / 2);
                rect.setHeight(option.rect.height() - 4);
                painter->setRenderHint(QPainter::Antialiasing);  //抗锯齿
                painter->setRenderHints(QPainter::SmoothPixmapTransform);  //平滑像素图变换
                painter->drawPixmap(rect, zi);
                painter->restore();
                handled = true;
            }
            if (isDong) {
                painter->save();
                rect = option.rect;
                int l1[2] = { 0, fu.width() / 2 + 4 };
                int l2[2] = { 0, zi.width() / 2 + 4 };
                int left = 0;
                left += rect.left();
                left += l1[isFu];
                left += l2[isZi];
                rect.setLeft(left);
                rect.setTopLeft(QPoint(rect.x(), rect.y() + 2));
                rect.setWidth(dong.width() / 2);
                rect.setHeight(option.rect.height() - 4);
                painter->setRenderHint(QPainter::Antialiasing);  //抗锯齿
                painter->setRenderHints(QPainter::SmoothPixmapTransform);  //平滑像素图变换
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

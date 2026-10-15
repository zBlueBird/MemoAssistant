#include "ui/IconFactory.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <functional>

namespace {
QIcon drawIcon(const std::function<void(QPainter &, const QRectF &)> &draw)
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    draw(painter, QRectF(4, 4, 56, 56));
    return QIcon(pixmap);
}
}

QIcon IconFactory::applicationIcon()
{
    return drawIcon([](QPainter &painter, const QRectF &rect) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(QStringLiteral("#4F7CFF")));
        painter.drawEllipse(rect);

        QPainterPath note;
        note.addRoundedRect(QRectF(19, 15, 27, 34), 5, 5);
        painter.setBrush(Qt::white);
        painter.drawPath(note);

        QPen linePen(QColor(QStringLiteral("#4F7CFF")), 3, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(linePen);
        painter.drawLine(QPointF(25, 25), QPointF(40, 25));
        painter.drawLine(QPointF(25, 33), QPointF(40, 33));
        painter.drawLine(QPointF(25, 41), QPointF(35, 41));
    });
}

QIcon IconFactory::addIcon()
{
    return drawIcon([](QPainter &painter, const QRectF &) {
        QPen pen(QColor(QStringLiteral("#315FD6")), 6, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(QPointF(32, 15), QPointF(32, 49));
        painter.drawLine(QPointF(15, 32), QPointF(49, 32));
    });
}

QIcon IconFactory::editIcon()
{
    return drawIcon([](QPainter &painter, const QRectF &) {
        QPen pen(QColor(QStringLiteral("#526071")), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        painter.drawLine(QPointF(17, 46), QPointF(42, 21));
        painter.drawLine(QPointF(38, 17), QPointF(47, 26));
        painter.drawLine(QPointF(17, 46), QPointF(29, 43));
    });
}

QIcon IconFactory::deleteIcon()
{
    return drawIcon([](QPainter &painter, const QRectF &) {
        QPen pen(QColor(QStringLiteral("#D95252")), 5, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(QPointF(21, 22), QPointF(24, 49));
        painter.drawLine(QPointF(43, 22), QPointF(40, 49));
        painter.drawLine(QPointF(18, 18), QPointF(46, 18));
        painter.drawLine(QPointF(27, 13), QPointF(37, 13));
    });
}

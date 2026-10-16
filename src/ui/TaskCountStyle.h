#pragma once

#include "models/DailyTaskSummary.h"

#include <QColor>
#include <QtGlobal>

namespace TaskCountStyle {

inline QColor mix(const QColor &start, const QColor &end, qreal amount)
{
    amount = qBound(0.0, amount, 1.0);
    return QColor::fromRgbF(start.redF() + (end.redF() - start.redF()) * amount,
                            start.greenF() + (end.greenF() - start.greenF()) * amount,
                            start.blueF() + (end.blueF() - start.blueF()) * amount);
}

inline qreal incompleteRatio(const DailyTaskSummary &summary)
{
    return summary.total > 0
        ? qBound(0.0, static_cast<qreal>(summary.incomplete) / summary.total, 1.0)
        : 0.0;
}

inline QColor badgeBackground(const DailyTaskSummary &summary)
{
    if (summary.incomplete <= 0) {
        return QColor(QStringLiteral("#E5EAF2"));
    }
    if (summary.incomplete >= summary.total) {
        return QColor(QStringLiteral("#C62828"));
    }
    return mix(QColor(QStringLiteral("#FFD166")),
               QColor(QStringLiteral("#FF7A00")),
               incompleteRatio(summary));
}

inline QColor badgeForeground(const DailyTaskSummary &summary)
{
    if (summary.incomplete <= 0) {
        return QColor(QStringLiteral("#6F7C8F"));
    }
    return summary.incomplete >= summary.total
        ? QColor(Qt::white)
        : QColor(QStringLiteral("#5C2B00"));
}

}

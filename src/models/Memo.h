#pragma once

#include <QDateTime>
#include <QString>

struct Memo
{
    qint64 id = 0;
    QString title;
    QString content;
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime reminderAt;
    QDateTime completedAt;
    bool completed = false;
    bool reminderSent = false;
};

Q_DECLARE_METATYPE(Memo)

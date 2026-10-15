#pragma once

#include "models/Memo.h"

#include <QWidget>

class QCheckBox;
class QLabel;

class MemoItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemoItemWidget(const Memo &memo, QWidget *parent = nullptr);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void completionChanged(qint64 id, bool completed);
    void detailsRequested(qint64 id);
    void editRequested(qint64 id);
    void deleteRequested(qint64 id);

private:
    Memo m_memo;
    QCheckBox *m_completedCheck;
    QLabel *m_titleLabel;
};

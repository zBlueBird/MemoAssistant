#pragma once

#include "models/Memo.h"

#include <QDialog>

class QCheckBox;
class QDateEdit;
class QLineEdit;
class QTimeEdit;
class QTextEdit;

class MemoEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MemoEditorDialog(QWidget *parent = nullptr);

    void setScheduledDate(const QDate &date);
    void setMemo(const Memo &memo);
    Memo memo() const;

private slots:
    void validateAndAccept();
    void updateReminderState(bool enabled);

private:
    Memo m_memo;
    QLineEdit *m_titleEdit;
    QTextEdit *m_contentEdit;
    QDateEdit *m_scheduledDateEdit;
    QCheckBox *m_reminderCheck;
    QDateEdit *m_reminderDateEdit;
    QTimeEdit *m_reminderTimeEdit;
};

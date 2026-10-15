#pragma once

#include <QDialog>

class QLabel;

class ReminderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReminderDialog(QWidget *parent = nullptr);
    void setMessage(const QString &message);

private:
    QLabel *m_messageLabel;
};

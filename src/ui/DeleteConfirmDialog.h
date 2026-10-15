#pragma once

#include <QDialog>

class DeleteConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteConfirmDialog(QWidget *parent = nullptr);
};

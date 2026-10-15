#pragma once

#include "models/Memo.h"

#include <QDialog>

class MemoDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MemoDetailsDialog(const Memo &memo, QWidget *parent = nullptr);
};
